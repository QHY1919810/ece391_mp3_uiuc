#include "tests.h"


#define PASS 1
#define FAIL 0

static uint8_t test_user_buffer[MAX_TERMINAL_BUF_CHARACTERS];  //this is a user buffer, which is used to communicate with system call


/* format these macros as you see fit */
#define TEST_HEADER 	\
	printf("[TEST %s] Running %s at %s:%d\n", __FUNCTION__, __FUNCTION__, __FILE__, __LINE__)
#define TEST_OUTPUT(name, result)	\
	printf("[TEST %s] Result = %s\n", name, (result) ? "PASS" : "FAIL")

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

	if ((idt[0x21].offset_15_00 == NULL) && 
		(idt[0x21].offset_31_16 == NULL)){
		assertion_failure();
		result = FAIL;
	}

	if ((idt[0x28].offset_15_00 == NULL) && 
		(idt[0x28].offset_31_16 == NULL)){
		assertion_failure();
		result = FAIL;
	}

	return result;
}

/* Page Test
 * 
 * DESCRIPTION: Test the value of CR and presence of pages
 * INPUTS: None
 * OUTPUTS: PASS/FAIL
 * SIDE EFFECTS: None
 * COVERAGE: Values in paging structures, values in CR and dereferencing different address
 * FILES: page.h/c
 */
int page_test() {
	TEST_HEADER;

	int i;
	char val;
	char* ptr;
	unsigned long reg_temp;
	int result = PASS;

	/* Check CR0 */
	asm volatile ("				\n\
		movl %%cr0, %0			\n\
		"
		: "=r" (reg_temp)
		:
		: "memory", "cc"
	);
	printf("CR0: %#x\n", reg_temp);
	if ((reg_temp & 0x80000001) != 0x80000001) {
		result = FAIL;
		printf("CR0 incorrect\n");
	}

	/* Check CR3 */
	asm volatile ("				\n\
		movl %%cr3, %0			\n\
		"
		: "=r" (reg_temp)
		:
		: "memory", "cc"
	);
	printf("CR3: %#x\n", reg_temp);
	if ((void*)reg_temp != (void*)pd) {
		result = FAIL;
		printf("CR3 incorrect\n");
	}

	/* Check CR4 */
	asm volatile ("				\n\
		movl %%cr4, %0			\n\
		"
		: "=r" (reg_temp)
		:
		: "memory", "cc"
	);
	printf("CR4: %#x\n", reg_temp);
	if ((reg_temp & 0x00000090) != 0x00000090) {
		result = FAIL;
		printf("CR4 incorrect\n");
	}

	/* Check presence of page directory */
	for (i = 0; i < NUM_PDE; i++) {
		/* page table of video memory */
		if (i == 0) {
			if (!pd[i].pde_table.present) {
				result = FAIL;
				printf("Page table of video memory not set\n");
			}
		}

		/* Kernel page */
		else if (i == 1) {
			if (!pd[i].pde_4m.present || !pd[i].pde_4m.global) {
				result = FAIL;
				printf("Kernel page not set\n");
			}
		}

		/* Other pages */
		else {
			if (pd[i].pde_table.present) {
				result = FAIL;
				printf("Page incorrectly set\n");
			}
		}
	}

	/* Check presence of page table */
	for (i = 0; i < NUM_PTE; i++) {
		/* Video memory page */
		if (i == VIDEO_INDEX) {
			if (!pt_video[i].present) {
				result = FAIL;
				printf("Video memory page not set\n");
			}
		}

		/* Other pages */
		else {
			if (pt_video[i].present) {
				result = FAIL;
				printf("Page incorrectly set\n");
			}
		}
	}

	/* Try to access content of each page */
	ptr = (char*)VIDEO;
	val = *ptr;
	ptr = (char*)(1 << 22); /* Kernel page at 4MB */
	val = *ptr;

	return result;
}


/* Page Fault Test
 * 
 * DESCRIPTION: Test the response of dereferencing unavailable address
 * INPUTS: None
 * OUTPUTS: PASS/FAIL
 * SIDE EFFECTS: None
 * COVERAGE: Page fault in IDT
 * FILES: page.h/c  idt.h/c  idt_handler.h/c
 */
int page_fault_test() {
	TEST_HEADER;

	char val;
	char* ptr;
	int result = FAIL;

	ptr = (char*)(VIDEO - (1 << 12)); /* Page before video memory */
	val = *ptr;

	return result;
}


/* 
 * deference_null
 * 	DESCRIPTION:
 * 		Dereference NULL ptr explicitly to test page fault (kind of a replication to page_fault_test above)
 * 		Ideally, on excuting the function, os should go into exception 0x0E and further interrupts are blocked.
 * 	INPUTS: none
 *  OUTPUTS: none
 *  SIDE EFFECTS: smashes any running user program and blocks further interrupts
 */
void dereference_null() {
	int32_t* ptr;
	ptr = NULL;
	
	*ptr = 1;	/* 1 is just a random number to be saved into memory pointed by ptr */
}

/* 
 * divide_by_zero
 * 	DESCRIPTION:
 *		Explicitly divide by zero to test exception 0x00.
 * 	INPUTS: none
 *  OUTPUTS: none
 *  SIDE EFFECTS: smashes any running user program and blocks further interrupts
 */
void divide_by_zero() {
	int32_t i; /* dividend */
	int32_t j; /* divisor */

	i = 1;
	j = 0;	/* use a variable to workaround warning */

	i /= j;
}

/* Checkpoint 2 tests */
/* 
 * rtc_open_close_test()
 * 	DESCRIPTION:
 * 		firts initial the rtc, and call the open & close function. Open will set freq to 2.
 * 	INPUTS: none
 *  OUTPUTS: none
 *  SIDE EFFECTS: print freq 2 on the screen.
 */
int rtc_open_close_test(){
	int ret = 0;
	TEST_HEADER;
	rtc_init();
	ret += rtc_open(NULL);
	ret += rtc_close(NULL);
	if(ret == 0){
		ret = PASS;
	}else{
		ret = FAIL;
	} 
	return ret;
}
/* 
 * rtc_read_test()
 * 	DESCRIPTION:
 * 		firts initial the rtc, then call rtc_read to wait for next interrupt.
 * 	INPUTS: none
 *  OUTPUTS: none
 *  SIDE EFFECTS: none
 */
int rtc_read_test(){
	int ret;
	rtc_init();
	printf("Wait for new interrupt\n");
	ret = rtc_read(NULL,NULL,NULL);
	printf("\n");
	printf("New interrupt has come");
	if(ret == 0){
		ret = PASS;
	}else{
		ret = FAIL;
	}
	return ret;
}
/* 
 * rtc_write_test()
 * 	DESCRIPTION:
 * 		set the freq of RTC.
 * 	INPUTS: none
 *  OUTPUTS: none
 *  SIDE EFFECTS: none
 */
int rtc_write_test(){
	rtc_init();
	int ret = 0;
	int freq;
	int j;
	for(freq = 2;freq <= 1024; freq = freq *2){
		ret += rtc_write(NULL,&freq,NULL);
		for(j = 0; j < freq;j++){
			ret += rtc_read(NULL,NULL,NULL);
		}
		printf("\n");
	}
	rtc_init();
	if(ret == 0) {
		return PASS;
	} else {
		return FAIL;
	}
}

/*
 * fs_dir_test
 * 	DESCRIPTION:
 * 		Test directory disk functions: open, read, write and close. 
 * 		The tests dumps the content under directory "." as a simulation for "ls".
 * 	INPUTS: directory name
 * 	RETURN VALUES: none
 */
int fs_dir_test(int8_t* dirname) {
	
	int32_t cnt;
	uint8_t buf[FILE_NAME_LENGTH + 1];

	printf("\n****** DIRECTORY READ TEST ******\n");
	printf("**** Reading %s ****\n", dirname);

	/* test directory open */
	if (dir_open((uint8_t*) dirname) == -1) {
		printf("Open Dir Failed\n");
		return FAIL;
	}

	/* test directory read */
	while (0 != (cnt = dir_read(0, buf, FILE_NAME_LENGTH+1))){
		if (cnt == -1) {
			printf("Read Data Failed\n");
			return FAIL;
		}		

		if (cnt != (strlen((const char*)buf) + 1)){
			printf("%d, %d, %s\n", cnt, strlen((const char*)buf) + 1, buf);
			return FAIL;
		}
		printf("%s ", buf);
	}
	printf("\n");

	/* test write functionality, input is given randomly */
	if (dir_write(0, "abcd", 4) != -1)
		return FAIL;

	if (dir_close(0) == -1) 
		printf("close failed\n");
		return FAIL;

	return PASS;
}

/*
 * fs_file_test
 * 	DESCRIPTION:
 * 		Test file disk functions: open, read, write and close. 
 * 	INPUTS: filename to be read
 * 	RETURN VALUES: none
 */
int fs_file_test(int8_t* filename) {

	int32_t cnt, i;
	uint8_t text[6500];

	printf("\n****** FILE READ TEST ******\n");
	printf("**** Reading %s ****\n", filename);
	/* test read file */
	if (file_open((uint8_t*) filename)) {
		printf("Open File Failed\n");
		return FAIL;
	}

	cnt = file_read(0, text, 6500);
	for (i = 0; i < cnt; i++) {
		if ((text[i] >= 32 && text[i] <= 126) || text[i] == 10 || text[i] == 13) {
			putc(text[i]);
		}
	}

	/* test write functionality, input is given randomly */
	if (file_write(0, "abcd", 4) != -1)
		return FAIL;

	if (file_close(0) == -1) 
		return FAIL;
		
	return PASS;
}




/* 
 * terminal_test
 * 	DESCRIPTION:
 *		Test the terminal read and terminal write as well as other terminal functions
 * 	INPUTS: none
 *  OUTPUTS: none
 *  SIDE EFFECTS: 
 */
void terminal_test(){
	int32_t i = 0;	
	terminal_open(NULL);
	printf("####### TERMINAL OPENED! #######\n ");
	while(i < 10){
		printf("\n####### NOW TEST TERMINAL READ #######\n ");
		printf("####### SIMPLY ENTER SOMETHING #######\n \n");
		update_cursor();
		terminal_read(0, test_user_buffer, 128);
		printf("\n####### NOW TEST TERMINAL WRITE #######\n ");
		printf("#### THIS IS WHAT WE ENTERED BEFORE ####\n \n");
		terminal_write(0, test_user_buffer, 128);
		i++;
	}
	terminal_close(NULL);
	printf("\n####### TERMINAL CLOSED! #######\n ");

}


/* checkpoint 3 tests */
/* 
 * syscall_file_op_test
 * 	DESCRIPTION:
 *		Test open, read, write, close system call (system call asmlinkage is not tested).
 * 	INPUTS: none
 *  OUTPUTS: Pass -- success
 * 			 Fail -- not pass
 */
int syscall_file_op_test() {
    TEST_HEADER;

    int result = PASS;
	int i;

    /* adding a fake pcb to make sure pointers are valid */
	pcb_t* fake_pcb = create_pcb(allocate_pid());

	for (i = 0; i < FD_ARRAY_SIZE; i++) {
		fake_pcb->file_desc_array[i].flags = 0;
		fake_pcb->file_desc_array[i].file_position = 0;
	}
	
	
    fake_pcb->file_desc_num = 2; /* Assume initial 2 is for stdin and stdout */
    fake_pcb->file_desc_array[0].flags = 0 | FD_FLAG_PRESENT;
    fake_pcb->file_desc_array[1].flags = 0 | FD_FLAG_PRESENT;
	
	printf("Testing Basic Functionality (frame1.txt): open -> read -> write -> close \n");
    /* Test for fd under open syscall */
    int fd = 0;
    fd = open((uint8_t*)"frame1.txt");
    if (fd == -1) {
        printf("open failed");
        result = FAIL;
    }
    if(fake_pcb->file_desc_num != 3 /* 2+1 (origin 2 + open 1 */) {
        result = FAIL;
        printf("incorrect opened file num after open");
    }

    /* Test for fd under read/write syscall */
    int32_t tmp_buf_size = 200; /* Just set for a buffer size */
    uint8_t buf[tmp_buf_size];
    if(read(fd, buf, tmp_buf_size) == -1){
        result = FAIL;
        printf("read failed");
    }
	printf("%s ", buf);

	while (read(fd, buf, tmp_buf_size) > 0) {
		printf("%s ", buf);
	}

    if(write(fd, buf, tmp_buf_size) != -1) {
        result = FAIL;
        printf("write failed");
    }

	
	printf("Testing Open under incorrect file name (should be covered in CH2)\n");
    /* Test for fd under close syscall */
    if(close(fd) == -1 || fake_pcb->file_desc_num != 2) {
        printf("close failed: file_desc_num = %d\n", fake_pcb->file_desc_num);
        result = FAIL;
    }

	/* Test for unexist file */
    if (open((uint8_t*)"IamaDumAss") != -1) {
        result = FAIL;
        printf("IamaDumAss fail");
    }
    if (open((uint8_t*)"frame1.tx") != -1) {
        result = FAIL;
        printf("frame1.tx fail");
    }
    if (open((uint8_t*)"") != -1) {
        result = FAIL;
        printf("empty fail");
    }
    /* Test for bad input */
    if (open((uint8_t*)NULL) != -1) {
        result = FAIL;
        printf("bad input fail");
    }

	printf("Testing incorrect fd (not valid | unopened files)\n");


	if (read(-1, (unsigned char *)buf, tmp_buf_size) != -1 )
        result = FAIL;
    if (write(-1, (unsigned char *)buf, tmp_buf_size) != -1 )
        result = FAIL;

    /**
     * Test for read/write syscall for big fd
     */
    if (read(FD_ARRAY_SIZE, (unsigned char *)buf, tmp_buf_size) != -1 )
        result = FAIL;
    if (write(FD_ARRAY_SIZE, (unsigned char *)buf, tmp_buf_size) != -1 )
        result = FAIL;

    /**
     * Test for read/write syscall for unopened fd
     */
    if (read(FD_ARRAY_SIZE - 1, (unsigned char *)buf, tmp_buf_size) != -1 )
        result = FAIL;
    if (write(FD_ARRAY_SIZE - 1, (unsigned char *)buf, tmp_buf_size) != -1 )
        result = FAIL;

    /**
     * Test bad input for buf
     */
	fd = open((uint8_t*)"frame0.txt");
    if (read(fd, (unsigned char *)NULL, tmp_buf_size) != -1 )
        result = FAIL;
    if (write(fd, (unsigned char *)NULL, tmp_buf_size) != -1 )
        result = FAIL;
	close(fd);

    /**
     * Test for close file descripor which is not present
     */
    if (close(NULL) != -1 )
        result = FAIL;
    if (close(123) != -1 )
        result = FAIL;
    if (close(FD_ARRAY_SIZE - 1) != -1 )
        result = FAIL;

    /* Reset task states */
    return result;
}


/* 
 * execute_test
 * 	DESCRIPTION:
 *		Test if we can successfully run execute.
 * 	INPUTS: none
 *  OUTPUTS: Pass -- success
 * 			 Fail -- not pass
 */
int execute_test(){
	int result;
	printf("Begin testing ...\n");

    if (execute((uint8_t*)"shell") == -1){
		printf("execute fails!\n");
        result = FAIL;
	}
	
	while(1){
		if (result == FAIL){
            printf("test fails!!\n");
		}else{
			printf("test pass!!\n");
		}
	}
	return result;
}

/* Checkpoint 4 tests */
/* Checkpoint 5 tests */


/* Test suite entry point */
void launch_tests(){
	/* idt test */
	// TEST_OUTPUT("idt_test", idt_test());
	/* paging test */
	TEST_OUTPUT("page_test", page_test());

	/* exception test */
	// page_fault_test();
	// dereference_null();
	// divide_by_zero();
	// TEST_OUTPUT("fs_dir_test: read directory", fs_dir_test("."));
    // execute((uint8_t*)"shell");

	/* Checkpoint 2 Tests */
    // terminal_test();
	// TEST_OUTPUT("rtc_read_test",rtc_read_test());
	// TEST_OUTPUT("rtc_open_close_test",rtc_open_close_test());
	// TEST_OUTPUT("fs_dir_test: read non-exist", fs_dir_test("non exist dir"));
	// TEST_OUTPUT("fs_dir_test: read file", fs_dir_test("frame0.txt"));
	// // TEST_OUTPUT("fs_file_test", fs_file_test("sigtest"));
	// TEST_OUTPUT("fs_file_test: read file", fs_file_test("frame0.txt"));
	// TEST_OUTPUT("fs_file_test: read non-exist", fs_file_test("non exist file"));
	// TEST_OUTPUT("fs_file_test: read directory", fs_file_test("."));

	// TEST_OUTPUT("rtc_write_test",rtc_write_test());

	/* checkpoint 3 tests */
	// TEST_OUTPUT("syscall file op test:", syscall_file_op_test());
}
