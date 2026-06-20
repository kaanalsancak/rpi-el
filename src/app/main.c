#include <stdio.h>
#include <sys/utsname.h>

int main(void)
{
    struct utsname system_info;

    if (uname(&system_info) != 0)
    {
        perror("uname failed");
        return 1;
    }

    printf("Raspberry Pi Embedded Linux Lab\n");
    printf("System name : %s\n", system_info.sysname);
    printf("Node name   : %s\n", system_info.nodename);
    printf("Release     : %s\n", system_info.release);
    printf("Machine     : %s\n", system_info.machine);

    return 0;
}
