#include "tests.h"
#include "x86_desc.h"
#include "lib.h"
#include "paging.h"
#include "file_system.h"
#include "rtc.h"
#include "terminal.h"

#define PASS 1
#define FAIL 0

/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL");

int rtc_test_flag = 0;
	
static inline void assertion_failure(){
	/* Use exception #15 for assertions, otherwise
	   reserved by Intel */
	asm volatile("int $15");
}


/* Checkpoint 1 tests */

/* IDT Test - Example
 * 
 * Asserts that first 10 IDT entries are not NULL
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: Load IDT, IDT definition
 * Files: x86_desc.h/S
 */
int idt_test(){
	TEST_HEADER;

	int i;
	int result = PASS;
	for (i = 0; i < 10; ++i){
		if ((idt[i].offset_15_00 == NULL) && 
			(idt[i].offset_31_16 == NULL)){
			assertion_failure();
			result = FAIL;
		}
	}

	return result;
}

// add more tests here


/* paging Test 
 * 
 * Prints out various memory addresses and check if there is page fault for invalid range paging address
 * Inputs: None
 * Outputs: PASS/FAIL, print
 * Side Effects: None
 * Coverage: none
 * Files: paging.c/h
 */
int paging_test(){
	TEST_HEADER;

	int result = PASS;
	
	result = check_1_paging(result);
	
	return result;
}

/* exception handler test
 * 
 * Asserts exception is correctly implemented by testing divide by zero
 * Inputs: None
 * Outputs: PASS/FAIL
 * Side Effects: None
 * Coverage: NONE
 * Files: idt.c/h
 */
int exception_handler_test(){
	TEST_HEADER;

	int result = PASS;

	// int a;
	
	// a=10/0;
	
	return result;
}


/* Checkpoint 2 tests */




/* file_read_test_frame1() 
 * 
 * find frame1.txt within directory "." and display on screen
 * Inputs: None
 * Outputs: print out frame1.txt
 * Side Effects: None
 * Coverage: none
 */
void file_read_test_frame1() {
	int8_t* filename;
	int i = 0;
	filename = "frame1.txt";
	if(file_open((uint8_t*)filename)==-1){
		printf("open failed\n");
		return;}
	uint8_t buf[173];
	
	int len;
	
	len = file_read(0,buf,173);
	
	//printf("len %d\n", len);
	for(i = 0; i<len ; i ++)
		//putc("c");
		putc(buf[i]);
}
/* file_read_test_frame0() 
 * 
 * find frame0.txt within directory "." and display on screen
 * Inputs: None
 * Outputs: print out frame0.txt
 * Side Effects: None
 * Coverage: none
 */
void file_read_test_frame0() {
	int8_t* filename;
	int i = 0;
	filename = "frame0.txt";
	if(file_open((uint8_t*)filename)==-1){
		printf("open failed\n");
		return;}
	uint8_t buf[185];
	
	int len;
	
	len = file_read(0,buf,185);
	if(len==0)
		printf("nothing to read");
	//printf("len %d\n", len);
	for(i = 0; i<len ; i ++)
		//putc("c");
		putc(buf[i]);
}

/* file_read_test_verylarge() 
 * 
 * find verylargetextwithverylongname.txt within directory "." and display on screen
 * Inputs: None
 * Outputs: print out verylargetextwithverylongname.txt
 * Side Effects: None
 * Coverage: none
 */
void file_read_test_verylarge() {
	int8_t* filename;
	int i = 0;
	filename = "verylargetextwithverylongname.txt";
	if(file_open((uint8_t*)filename)==-1){
		printf("open failed\n");
		return;}
	uint8_t buf[7000];
	int len;
	len = file_read(0,buf,7000);
	for(i = 0; i<len ; i ++)
		putc(buf[i]);
}

/* file_read_test_cat() 
 * 
 * find cat file within directory "." and display on screen
 * Inputs: None
 * Outputs: print out cat. some unknown and known symbols are present.
 * Side Effects: None
 * Coverage: none
 */
void file_read_test_cat() {
	int8_t* filename;
	int i = 0;
	filename = "cat";
	if(file_open((uint8_t*)filename)==-1){
		printf("open failed\n");
		return;}
	uint8_t buf[5445];
	int len;
	len = file_read(0,buf,5445);
	for(i = 0; i<len ; i ++)
		putc(buf[i]);
}

/* dir_write_test() 
 * 
 * if the return value of directory_write is -1. Then print out success.
 * Inputs: None
 * Outputs: print out dir write test success if directory_write function return value correctly.
 * Side Effects: None
 * Coverage: none
 */
void dir_write_test(){
	if(directory_write(0,(void*)0,0)==-1)
		printf("dir write test success\n");
}
/* dir_read_test() 
 * 
 * print out all the file name stored in dir_entr
 * Inputs: None
 * Outputs: print out all file names and file type stored in dir_entr
 * Side Effects: None
 * Coverage: none
 */

void dir_read_test() {
	int8_t* test;
	test= ".";
	directory_open((uint8_t*)test);
	uint8_t buf[34];
	int len;
	//int count=0;
	
	/*repeating loop until it reads all data stored in directory.*/
	while ((len=directory_read(0, buf, 32)) != -1) {
		int i;
		/*print out file name*/
		printf("file_name:");
		/*print out each file name characters*/
		for(i=0; i <len; i++){
		printf("%c", buf[i]);
		}
		/*print out space to align spaces*/
		for( i=len; i <32; i++){
		printf(" ");
		}
		/*print out file type*/
		printf("file_type:");
		printf("%d", buf[len]);
		printf("\n");
		//printf("file_size:");
		//printf("%d", buf[len+1]);
		//printf("\n");
	}
}

/* dir_open_test() 
 * 
 * print success or fail depending on the return value of directory_open.
 * Inputs: None
 * Outputs: print out success if directory_open return 0, fail if directory_open return -1;
 * Side Effects: None
 * Coverage: none
 */
 
void dir_open_test(){
	int8_t* test;
	test= ".";
	if(directory_open((uint8_t*)test)!=-1)
		printf("dir open test success\n");
	else	
		printf("dir open test failed");
}

/* dir_close_test() 
 * 
 * print success if directory_close function return 0
 * Inputs: None
 * Outputs: print out "dir close test success"
 * Side Effects: None
 * Coverage: none
 */
void dir_close_test(){
	if(!directory_close(0))
		printf("dir close test success\n");
}

/* rtc_test
 * test rtc read and write
 * Inputs: None
 * Outputs: PASS
 * Side Effects: changes the frequency
 * Files: rtc.c/h
 */
int rtc_test(){
	TEST_HEADER;
	
	int result = PASS;
	rtc_test_flag = 1;
	
	int32_t check;
	int32_t check_read;
	int32_t i;
	int32_t time;
	int32_t sec;
	
	int32_t buf[16] = {2, 4, 8, 16, 32, 64, 128, 256, 512, 1024, 2048, 4096, 8192, 2, 5, 16384};	
	
	for(i = 0; i < 16; i++){
		putc('\n');
		time = 0;
		sec = 0;
		check = rtc_write(0, &buf[i], 4);
		if (check == 0){
			printf("Write success, set frequency to: %d HZ \n", buf[i]);
		}
		else{
			printf("Write failed, set frequency to: %d HZ \n", buf[i]);
		}
		
		check_read = rtc_read(0, NULL, 4);
		if(check_read == 0){
			printf("Reading Success");
		}
		
		while(time < 0xFFFFFFF){
			sec += 1;
			if(sec == 1){
				time += 1;
				sec = 0;
			}
		}
	}
	
	rtc_test_flag = 0;	
	printf("\n");
	return result;
}

/*
terminal_read_test: this tests our terminal read function by
having the user input a string and pressing enter and then printing the keyboard buffer
INPUTS: NONE
OUTPUTS: RESULT
*/
int terminal_read_test()
{
	TEST_HEADER;
	
	int result = PASS; //result
	int nbytes = 0; //nbytes is the number of bytes that we read
	char buffer[128];
	int i = 0;
	
	puts("Please input a string and then press enter\n");
	while(nbytes == 0) //this loop is used to constantly check if there is a newline in the keyboard buffer
	{
		nbytes = terminal_read(10, buffer, 128);
	}
	putc('\n');
	while(i < nbytes) //we print the keyboard buffer
	{
		putc(buffer[i]);
		i++;
	}
	putc('\n');
	printf("%d bytes were written", nbytes);
	return result; //return the result
}

/*
terminal_write_test: this tests our terminal_write function by printing a string and testing a NULL pointer
INPUTS: NONE
OUTPUTS: RESULT
*/
int terminal_write_test()
{
	TEST_HEADER;
	int result = PASS;
	char message[10] = {'!', '?', 0, 'T', 'E', 'S', 'T', '0', 0, '\n'}; //this is the string that we have to print
	if(terminal_write(10, message, 10) != 10) //we want to make sure that it prints the correct amount of bytes
	{
		result = FAIL;
	}
	if(terminal_write(10, NULL, 10) != -1) //we want it to return -1 if we give an invalid pointer
	{
		result = FAIL;
	}
	return result;
}

/* Checkpoint 3 tests */
/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	//TEST_OUTPUT("idt_test", idt_test());
	//TEST_OUTPUT("paging_test", paging_test());
	//TEST_OUTPUT("paging_test", exception_handler_test());
	//file_read_test_frame1();

	//printf("\n");
	file_read_test_frame0();
	//file_read_test_verylarge();
	//file_read_test_cat()
	// launch your tests here
    //dir_open_test();
	//dir_read_test();
	//terminal_read_test();
	//terminal_write_test();
	
	//TEST_OUTPUT("TERMINAL WRITE TEST", terminal_write_test());
	//TEST_OUTPUT("TERMINAL READ TEST", terminal_read_test());
	


	//TEST_OUTPUT("rtc_test", rtc_test());
	
	// launch your tests here
	
}
