/*
	This file is needed for setjmp() and longjmp() from libc_setjmp to compile

	Assembly for these functions calls _sigprocmask to read/restore signal masks.
	Since we don't have signals in phantom, we don't do anything in this function.
*/

int _sigprocmask(int how, void *set, void *oldset)
{
	return 0;
}
