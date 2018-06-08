
// central.c - communication with central server

#include "qwsvdef.h"
#include <curl/curl.h>

#define GENERATE_CHALLENGE_PATH     "Authentication/GenerateChallenge"
#define VERIFY_RESPONSE_PATH        "Authentication/VerifyResponse"
#define CHECKIN_PATH                "ServerApi/Checkin"
#define MIN_CHECKIN_PERIOD          60

#ifdef _WIN32
#pragma comment(lib, "libcurld.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")
#endif

static cvar_t sv_www_address = { "sv_www_address", "" };
static cvar_t sv_www_authkey = { "sv_www_authkey", "" };
static cvar_t sv_www_checkin_period = { "sv_www_checkin_period", "60" };

static CURLM* curl_handle = NULL;

static double last_checkin_time;

#define MAX_RESPONSE_LENGTH    (4096*2)

struct web_request_data_s;

typedef void(*web_response_func_t)(struct web_request_data_s* req, qbool valid);
static void Web_ConstructURL(char* url, const char* path, int sizeof_url);
static void Web_SubmitRequestForm(const char* url, struct curl_httppost *first_form_ptr, struct curl_httppost *last_form_ptr, web_response_func_t callback, const char* requestId, void* internal_data);

typedef struct web_request_data_s {
	CURL*               handle;
	double              time_sent;

	// response from server
	char                response[MAX_RESPONSE_LENGTH];
	size_t              response_length;

	// form data
	struct curl_httppost *first_form_ptr;
	struct curl_httppost *last_form_ptr;

	// called when response complete
	web_response_func_t onCompleteCallback;
	void*               internal_data;
	char*               request_id;                 // if set, content will be passed to game-mod. will be Q_free()'d

	struct web_request_data_s* next;
} web_request_data_t;

static int utf8Encode(char in, char* out1, char* out2) {
	if (in <= 0x7F) {
		// <= 127 is encoded as single byte, no translation
		*out1 = in;
		return 1;
	}
	else {
		// Two byte characters... 5 bits then 6
		*out1 = 0xC0 | ((in >> 6) & 0x1F);
		*out2 = 0x80 | (in & 0x3F);
		return 2;
	}
}

static web_request_data_t* web_requests;

static qbool CheckFileExists(const char* path)
{
	FILE* f;
	if (!(f = fopen(path, "rb"))) {
		return false;
	}
	fclose(f);
	return true;
}

size_t Web_StandardTokenWrite(void* buffer, size_t size, size_t nmemb, void* userp)
{
	web_request_data_t* data = (web_request_data_t*)userp;
	size_t available = sizeof(data->response) - data->response_length;
	if (size * nmemb > available) {
		Con_DPrintf("WWW: Response too large, size*nmemb = %d, available %d\n", size * nmemb, available);
		return 0;
	}
	else if (size * nmemb > 0) {
		Con_DPrintf("WWW: response received, writing %d bytes\n", size * nmemb);
		memcpy(data->response + data->response_length, buffer, size * nmemb);
		data->response_length += size * nmemb;
		Con_DPrintf("WWW: response_length now %d bytes\n", data->response_length);
	}
	return size * nmemb;
}

typedef struct response_field_s {
	const char* name;
	const char** value;
} response_field_t;

static int ProcessWebResponse(web_request_data_t* req, response_field_t* fields, int field_count)
{
	char* colon, *newline;
	char* start = req->response;
	int i;
	int total_fields = 0;

	// Response should be multiple lines, <Key>:<Value>\n
	// For multi-line values, prefix end of line with $
	req->response[req->response_length] = '\0';
	while ((colon = strchr(start, ':'))) {
		newline = strchr(colon, '\n');
		if (newline) {
			while (newline && newline != colon + 1 && *(newline - 1) == '$') {
				*(newline - 1) = ' ';
				newline = strchr(newline + 1, '\n');
			}
			if (newline) {
				*newline = '\0';
			}
		}

		*colon = '\0';
		for (i = 0; i < field_count; ++i) {
			if (!strcmp(start, fields[i].name)) {
				*fields[i].value = colon + 1;
			}
		}

		if (newline == NULL) {
			break;
		}

		start = newline + 1;
		while (*start && (*start == 10 || *start == 13)) {
			++start;
		}
		++total_fields;
	}

	return total_fields;
}

void Auth_GenerateChallengeResponse(web_request_data_t* req, qbool valid)
{
	client_t* client = (client_t*) req->internal_data;
	const char* response = NULL;
	const char* challenge = NULL;
	const char* message = NULL;
	response_field_t fields[] = {
		{ "Result", &response },
		{ "Challenge", &challenge },
		{ "Message", &message }
	};

	if (client->login_request_time != req->time_sent) {
		// Ignore result, subsequent request sent
		return;
	}

	req->internal_data = NULL;
	ProcessWebResponse(req, fields, sizeof(fields) / sizeof(fields[0]));

	if (response && !strcmp(response, "Success") && challenge) {
		char buffer[128];
		strlcpy(buffer, "//challenge ", sizeof(buffer));
		strlcat(buffer, challenge, sizeof(buffer));
		strlcat(buffer, "\n", sizeof(buffer));

		strlcpy(client->challenge, challenge, sizeof(client->challenge));
		SV_ClientPrintf(client, PRINT_HIGH, "Challenge stored...\n", message);

		ClientReliableWrite_Begin (client, svc_stufftext, 2+strlen(buffer));
		ClientReliableWrite_String (client, buffer);
	}
	else if (message) {
		SV_ClientPrintf(client, PRINT_HIGH, "Error: %s\n", message);
	}
	else {
		// Maybe add CURLOPT_ERRORBUFFER?
		SV_ClientPrintf(client, PRINT_HIGH, "Error: unknown error\n");
	}
}

void Auth_ProcessLoginAttempt(web_request_data_t* req, qbool valid)
{
	client_t* client = (client_t*) req->internal_data;
	const char* response = NULL;
	const char* login = NULL;
	const char* preferred_alias = NULL;
	const char* message = NULL;
	response_field_t fields[] = {
		{ "Result", &response },
		{ "Alias", &preferred_alias },
		{ "Login", &login },
		{ "Message", &message }
	};

	req->internal_data = NULL;
	if (client->login_request_time != req->time_sent) {
		// Ignore result, subsequent request sent
		return;
	}

	ProcessWebResponse(req, fields, sizeof(fields) / sizeof(fields[0]));

	if (response && !strcmp(response, "Success")) {
		if (login) {
			char oldval[MAX_EXT_INFO_STRING];

			strlcpy(oldval, Info_Get(&client->_userinfo_ctx_, "*auth"), sizeof(oldval));
			strlcpy(client->login, login, sizeof(client->login));

			Info_SetStar(&client->_userinfo_ctx_, "*auth", login);
			ProcessUserInfoChange(client, "*auth", oldval);
		}

		if (!preferred_alias) {
			preferred_alias = login;
		}

		if (preferred_alias) {
			char oldval[MAX_EXT_INFO_STRING];
			strlcpy (oldval, client->name, MAX_EXT_INFO_STRING);

			// Change nickname
			SV_BroadcastPrintf(PRINT_HIGH, "%s logged in as %s\n", client->name, preferred_alias);
			Info_Set(&client->_userinfo_ctx_, "name", preferred_alias);

			ProcessUserInfoChange(client, "name", oldval);

			if (strcmp(preferred_alias, oldval)) {
				// Change name cvar in client
				MSG_WriteByte (&client->netchan.message, svc_stufftext);
				MSG_WriteString (&client->netchan.message, va("name %s\n", preferred_alias));
			}
		}
		client->logged = true;
	}
	else if (message) {
		SV_ClientPrintf(client, PRINT_HIGH, "Error: %s\n", message);
	}
	else {
		// Maybe add CURLOPT_ERRORBUFFER?
		SV_ClientPrintf(client, PRINT_HIGH, "Error: unknown error\n");
	}
}

void Web_PostResponse(web_request_data_t* req, qbool valid)
{
	if (valid) {
		const char* broadcast = NULL;
		const char* upload = NULL;
		const char* uploadPath = NULL;

		response_field_t fields[] = {
			{ "Broadcast", &broadcast },
			{ "UploadPath", &uploadPath },
			{ "Upload", &upload }
		};

		req->response[req->response_length] = '\0';

		Con_DPrintf("Response from web server:\n");
		Con_DPrintf(req->response);

		ProcessWebResponse(req, fields, sizeof(fields) / sizeof(fields[0]));

		if (broadcast) {
			// Server is making announcement
			SV_BroadcastPrintfEx(PRINT_HIGH, 0, "%s\n", broadcast);
		}
		if (upload && uploadPath) {
			if (strstr(uploadPath, "//") || FS_UnsafeFilename(upload)) {
				Con_Printf("Upload request deemed unsafe, ignoring...\n");
			}
			else {
				if (!strncmp(upload, "demos/", 6)) {
					// Ok - could be demo_dir, or full path
					char demoName[MAX_OSPATH];

					if (sv_demoDir.string[0]) {
						char url[512];
						struct curl_httppost *first_form_ptr = NULL;
						struct curl_httppost *last_form_ptr = NULL;

						snprintf(demoName, sizeof(demoName), "%s/%s/%s", fs_gamedir, sv_demoDir.string, upload + 6);
						if (!CheckFileExists(demoName) && sv_demoDirAlt.string[0]) {
							snprintf(demoName, sizeof(demoName), "%s/%s/%s", fs_gamedir, sv_demoDirAlt.string, upload + 6);
						}

						if (CheckFileExists(demoName)) {
							Web_ConstructURL(url, uploadPath, sizeof(url));

							curl_formadd(&first_form_ptr, &last_form_ptr,
								CURLFORM_PTRNAME, "file",
								CURLFORM_FILE, demoName,
								CURLFORM_END
							);

							curl_formadd(&first_form_ptr, &last_form_ptr,
								CURLFORM_COPYNAME, "localPath",
								CURLFORM_COPYCONTENTS, upload,
								CURLFORM_END
							);

							Web_SubmitRequestForm(url, first_form_ptr, last_form_ptr, Web_PostResponse, "upload", NULL);
							Con_Printf("Uploading %s...\n", demoName);
						}
						else {
							Con_Printf("Couldn't find file %s, ignoring...\n", demoName);
						}
					}
				}
				else {
					Con_Printf("Upload request folder not authorised, ignoring...\n");
				}
			}
		}
	}
	else {
		Con_Printf("Failure contacting central server.\n");
	}

	if (req->internal_data) {
		Q_free(req->internal_data);
	}
}

static void Web_SubmitRequestForm(const char* url, struct curl_httppost *first_form_ptr, struct curl_httppost *last_form_ptr, web_response_func_t callback, const char* requestId, void* internal_data)
{
	CURL* req = curl_easy_init();
	web_request_data_t* data = Q_malloc(sizeof(web_request_data_t));
	CURLFORMcode code = curl_formadd(&first_form_ptr, &last_form_ptr,
		CURLFORM_PTRNAME, "authKey",
		CURLFORM_COPYCONTENTS, sv_www_authkey.string,
		CURLFORM_END
	);

	data->onCompleteCallback = callback;
	data->time_sent = sv.time;
	data->internal_data = internal_data;
	data->handle = req;
	data->request_id = requestId && requestId[0] ? strdup(requestId) : NULL;
	data->first_form_ptr = first_form_ptr;

	curl_easy_setopt(req, CURLOPT_URL, url);
	curl_easy_setopt(req, CURLOPT_WRITEDATA, data);
	curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, Web_StandardTokenWrite);
	if (first_form_ptr) {
		curl_easy_setopt(req, CURLOPT_POST, 1);
		curl_easy_setopt(req, CURLOPT_HTTPPOST, first_form_ptr);
	}

	curl_multi_add_handle(curl_handle, req);
	data->next = web_requests;
	web_requests = data;
}

void Central_VerifyChallengeResponse(client_t* client, const char* challenge, const char* response)
{
	char url[512];
	struct curl_httppost *first_form_ptr = NULL;
	struct curl_httppost *last_form_ptr = NULL;
	CURLFORMcode code;

	code = curl_formadd(&first_form_ptr, &last_form_ptr,
		CURLFORM_PTRNAME, "challenge",
		CURLFORM_COPYCONTENTS, challenge,
		CURLFORM_END
	);

	code = curl_formadd(&first_form_ptr, &last_form_ptr,
		CURLFORM_PTRNAME, "response",
		CURLFORM_COPYCONTENTS, response,
		CURLFORM_END
	);

	Web_ConstructURL(url, VERIFY_RESPONSE_PATH, sizeof(url));

	client->login_request_time = sv.time;
	Web_SubmitRequestForm(url, first_form_ptr, last_form_ptr, Auth_ProcessLoginAttempt, NULL, client);
}

void Central_GenerateChallenge(client_t* client, const char* username)
{
	char url[512];
	struct curl_httppost *first_form_ptr = NULL;
	struct curl_httppost *last_form_ptr = NULL;
	CURLFORMcode code;

	Web_ConstructURL(url, GENERATE_CHALLENGE_PATH, sizeof(url));

	code = curl_formadd(&first_form_ptr, &last_form_ptr,
		CURLFORM_PTRNAME, "userName",
		CURLFORM_COPYCONTENTS, username,
		CURLFORM_END
	);

	client->login_request_time = sv.time;
	Web_SubmitRequestForm(url, first_form_ptr, last_form_ptr, Auth_GenerateChallengeResponse, NULL, client);
}

static void Web_ConstructURL(char* url, const char* path, int sizeof_url)
{
	strlcpy(url, sv_www_address.string, sizeof_url);
	if (url[strlen(url) - 1] != '/') {
		strlcat(url, "/", sizeof_url);
	}
	while (*path == '/') {
		++path;
	}
	strlcat(url, path, sizeof_url);
}

static void Web_SendRequest(qbool post)
{
	struct curl_httppost *first_form_ptr = NULL;
	struct curl_httppost *last_form_ptr = NULL;
	char url[512];
	char* requestId = NULL;
	int i;

	if (!sv_www_address.string[0]) {
		Con_Printf("Address not set - functionality disabled\n");
		return;
	}

	if (Cmd_Argc() < 3) {
		Con_Printf("Usage: %s <url> <request-id> (<key> <value>)*\n", Cmd_Argv(0));
		return;
	}

	Web_ConstructURL(url, Cmd_Argv(1), sizeof(url));

	requestId = Cmd_Argv(2);
	if (requestId[0]) {
		requestId = Q_strdup(requestId);
	}
	else {
		requestId = NULL;
	}

	for (i = 3; i < Cmd_Argc() - 1; i += 2) {
		char encoded_value[128];
		int encoded_length = 0;
		int j;
		char* name;
		char* value;
		CURLFORMcode code;

		name = Cmd_Argv(i);
		value = Cmd_Argv(i + 1);
		for (j = 0; j < strlen(value) && encoded_length < sizeof(encoded_value) - 2; ++j) {
			encoded_length += utf8Encode(value[j], &encoded_value[encoded_length], &encoded_value[encoded_length + 1]);
		}
		encoded_value[encoded_length] = '\0';

		code = curl_formadd(&first_form_ptr, &last_form_ptr,
			CURLFORM_COPYNAME, name,
			CURLFORM_COPYCONTENTS, encoded_value,
			CURLFORM_END
		);

		if (code != CURLE_OK) {
			curl_formfree(first_form_ptr);
			Con_Printf("Request failed\n");
			return;
		}
	}

	Web_SubmitRequestForm(url, first_form_ptr, last_form_ptr, Web_PostResponse, requestId, NULL);
}

static void Web_GetRequest_f(void)
{
	Web_SendRequest(false);
}

static void Web_PostRequest_f(void)
{
	Web_SendRequest(true);
}

static void Web_PostFileRequest_f(void)
{
	struct curl_httppost *first_form_ptr = NULL;
	struct curl_httppost *last_form_ptr = NULL;
	char* requestId = NULL;
	char url[512];
	char path[MAX_OSPATH];
	CURLFORMcode code = 0;
	const char* specified = Cmd_Argv(3);

	if (!sv_www_address.string[0]) {
		Con_Printf("Address not set - functionality disabled\n");
		return;
	}

	if (specified[0] == '*' && specified[1] == '\0') {
		const char* demoname = SV_MVDDemoName();

		if (!sv.mvdrecording || !demoname) {
			Con_Printf("Not recording demo!\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s/%s", fs_gamedir, sv_demoDir.string, SV_MVDName2Txt(demoname));
	}
	else {
		if (strstr(specified, ".cfg") || !strchr(specified, '/')) {
			Con_Printf("Filename invalid\n");
			return;
		}

		snprintf(path, MAX_OSPATH, "%s/%s", fs_gamedir, specified);
	}

	if (Cmd_Argc() < 4) {
		Con_Printf("Usage: %s <url> <request-id> <file> (<key> <value>)*\n", Cmd_Argv(0));
		return;
	}

	if (FS_UnsafeFilename(path)) {
		Con_Printf("Filename invalid\n");
		return;
	}

	Web_ConstructURL(url, Cmd_Argv(1), sizeof(url));

	requestId = Cmd_Argv(2);
	if (requestId[0]) {
		requestId = Q_strdup(requestId);
	}
	else {
		requestId = NULL;
	}

	if (! CheckFileExists(path)) {
		Con_Printf("Failed to open file\n");
		return;
	}

	code = curl_formadd(&first_form_ptr, &last_form_ptr,
		CURLFORM_PTRNAME, "file",
		CURLFORM_FILE, path,
		CURLFORM_END
	);

	Web_SubmitRequestForm(url, first_form_ptr, last_form_ptr, Web_PostResponse, requestId, NULL);
}

void Central_ProcessResponses(void)
{
	CURLMsg* msg;
	int running_handles = 0;
	int messages_in_queue = 0;
	qbool server_busy = false;

	if (!last_checkin_time) {
		last_checkin_time = curtime;
		return;
	}

	curl_multi_perform(curl_handle, &running_handles);

	server_busy = running_handles || GameStarted();

	while ((msg = curl_multi_info_read(curl_handle, &messages_in_queue))) {
		if (msg->msg == CURLMSG_DONE) {
			CURL* handle = msg->easy_handle;
			CURLcode result = msg->data.result;
			web_request_data_t** list_pointer = &web_requests;

			curl_multi_remove_handle(curl_handle, handle);

			while (*list_pointer) {
				web_request_data_t* this = *list_pointer;

				if (this->handle == handle) {
					// Remove from queue immediately
					*list_pointer = this->next;

					if (this->request_id && !strcmp(this->request_id, "upload")) {
						this = this;
					}
					if (this->onCompleteCallback) {
						if (result == CURLE_OK) {
							this->onCompleteCallback(this, true);
						}
						else {
							Con_DPrintf("ERROR: %s\n", curl_easy_strerror(result));
							this->onCompleteCallback(this, false);
						}
					}
					else {
						if (result != CURLE_OK) {
							Con_DPrintf("ERROR: %s\n", curl_easy_strerror(result));
						}
						else {
							Con_DPrintf("WEB OK, no callback\n");
						}
					}

					// free memory
					curl_formfree(this->first_form_ptr);
					Q_free(this->request_id);
					Q_free(this);

					break;
				}

				list_pointer = &this->next;
			}

			curl_easy_cleanup(handle);
		}
	}

	if (sv_www_address.string[0] && !server_busy && curtime - last_checkin_time > max(MIN_CHECKIN_PERIOD, sv_www_checkin_period.value)) {
		char url[512];

		Web_ConstructURL(url, CHECKIN_PATH, sizeof(url));

		Web_SubmitRequestForm(url, NULL, NULL, Web_PostResponse, NULL, NULL);

		last_checkin_time = curtime;
	}
	else if (server_busy) {
		last_checkin_time = curtime;
	}
}

void Central_Shutdown(void)
{
	if (curl_handle) {
		curl_multi_cleanup(curl_handle);
		curl_handle = NULL;
	}

	curl_global_cleanup();
}

void Central_Init(void)
{
	curl_global_init(CURL_GLOBAL_DEFAULT);

	Cvar_Register(&sv_www_address);
	Cvar_Register(&sv_www_authkey);
	Cvar_Register(&sv_www_checkin_period);

	curl_handle = curl_multi_init();

	if (curl_handle) {
		Cmd_AddCommand("sv_web_get", Web_GetRequest_f);
		Cmd_AddCommand("sv_web_post", Web_PostRequest_f);
		Cmd_AddCommand("sv_web_postfile", Web_PostFileRequest_f);
	}
}
