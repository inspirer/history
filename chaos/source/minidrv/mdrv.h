// selectors
#define mem_kernel_code  0x08
#define mem_idt_gdt_data 0x10
#define mem_page_catalog 0x18
#define mem_kernel_data  0x20
#define mem_all_data     0x28
#define mem_start_desc   0x30

// errors
#define err_no_free_desc     1
#define err_no_free_memory   2
#define err_no_support       3
#define err_no_free_LDT_desc 4
#define err_wrong_info       5
#define err_access_denied    6
#define err_too_much_tasks   7
#define err_not_killed       8
#define err_already_used     9
#define err_wrong_id         10
#define err_sys_integrity    11
#define err_wrong_regname    12
#define err_reg_key_has_sub  13
#define err_no_such_task     14
