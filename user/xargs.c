// user/xargs.c
#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/param.h"
#include "user/user.h"

// DEBUG 开关
#define DEBUG 0
#define debug(x) if(DEBUG){ x; }

char*
strdup(const char *s) {
    int n = strlen(s) + 1;
    char *p = malloc(n);
    if(p == 0) return 0;
    memmove(p, s, n);
    return p;
}

int
main(int argc, char *argv[])
{
    // printf("step to now");
    if (argc < 2) {
        fprintf(2, "Usage: xargs command [args...]\n");
        exit(1);
    }

    // base_args 保存用户给出的 command 及其固定参数（argv[1..argc-1]）
    char *base_args[MAXARG];
    int base_n = 0;
    for (int i = 1; i < argc && base_n < MAXARG-2; i++) {
        base_args[base_n++] = argv[i];
    }
    // 注意：后面我们会把行内容放在 base_args 末尾，所以这里不把 NULL 放入

    // 读取 stdin，逐行处理
    char linebuf[1024];
    int m = 0;
    while (1) {
        int r = read(0, &linebuf[m], 1);
        if (r < 0) {
            // read error
            fprintf(2, "xargs: read error\n");
            exit(1);
        } else if (r == 0) {
            // EOF: 如果缓冲区中有未结束的一行，也要处理它
            if (m > 0) {
                linebuf[m] = 0;
                // 处理这行
            } else {
                break; // 无剩余内容，退出循环
            }
        }

        if (r == 0 || linebuf[m] == '\n') {
            // 行结束（或者 EOF 到达且 m>0），构造 argv
            if (linebuf[m] == '\n') linebuf[m] = 0; // 替换换行符
            else linebuf[m] = 0; // EOF 未换行的情况

            // 构造实际 argv：base_args + this_line + NULL
            char *child_argv[MAXARG];
            int k = 0;
            // 先拷贝 base_args
            for (int i = 0; i < base_n && k < MAXARG-2; i++) {
                child_argv[k++] = base_args[i];
            }
            // 把整行作为最后一个参数
            child_argv[k++] = strdup(linebuf); // strdup 在 xv6 user 模式可用（实现是 malloc+strcpy）
            child_argv[k] = 0;

            debug(
                printf("DEBUG: exec %s with args:\n", child_argv[0]);
                for (int i = 0; child_argv[i]; i++) {
                    printf("  argv[%d]=%s\n", i, child_argv[i]);
                }
            )

            int pid = fork();
            if (pid < 0) {
                fprintf(2, "xargs: fork failed\n");
                free(child_argv[k-1]);
                exit(1);
            } else if (pid == 0) {
                // child: exec
                if (exec(child_argv[0], child_argv) < 0) {
                    // exec 失败
                    fprintf(2, "xargs: Error exec %s\n", child_argv[0]);
                    exit(1);
                }
                // 不会返回
            } else {
                // parent: wait
                wait(0);
                free(child_argv[k-1]); // 释放 strdup 的内存
            }

            // reset buffer
            m = 0;
            if (r == 0) break; // EOF case: we've handled the last line
        } else {
            // 继续读字符
            m++;
            if (m >= (int)sizeof(linebuf)) {
                fprintf(2, "xargs: arguments too long\n");
                exit(1);
            }
        }
    }

    exit(0);
}
