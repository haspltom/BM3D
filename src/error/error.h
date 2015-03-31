#ifndef ERROR_H
#define ERROR_H

typedef char* err_msg;
err_msg ptr;

extern void generate_error (char* msg);
extern void free_error_ptr ();

#endif // ERROR_H
