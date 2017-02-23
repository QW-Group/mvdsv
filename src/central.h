
#ifndef CENTRAL_H
#define CENTRAL_H

void Central_Init(void);
void Central_Shutdown(void);
void Central_ProcessResponses(void);
void Central_SubmitGame(const char* path);

#endif // !CENTRAL_H