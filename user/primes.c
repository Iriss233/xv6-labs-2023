#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Winfinite-recursion"

void
sieve(int left[2])
{
    close(left[1]);     // 关闭左 pipe 的写端

    int p;
    if (read(left[0], &p, sizeof(p)) == 0) {
        close(left[0]);
        exit(0);
    }

    printf("prime %d\n", p);

    int right[2];
    pipe(right);

    int pid = fork();
    if (pid == 0) {
        // child → 负责下一层筛选
        close(right[1]);     // child 不需要写
        close(left[0]);      // child 不再使用 left pipe
        sieve(right);
        exit(0);
    } else {
        // parent → 负责过滤 p 的倍数，写入 right
        close(right[0]);     // parent 不读右 pipe
        int x;

        while (read(left[0], &x, sizeof(x)) == sizeof(x)) {
            if (x % p != 0) {
                write(right[1], &x, sizeof(x));
            }
        }

        close(right[1]);     // 必须关闭，否则 child read 不会结束
        close(left[0]);      // 已经读完，也关掉

        wait(0);
        exit(0);
    }
}

int
main(int argc, char *argv[])
{
    int p[2];
    pipe(p);

    int pid = fork();
    if (pid == 0) {
        close(p[1]);     // child 不写
        sieve(p);        // 开始筛
        exit(0);
    } else {
        close(p[0]);     // parent 不读

        for (int i = 2; i <= 35; i++)
            write(p[1], &i, sizeof(i));

        close(p[1]);     //！！！务必关闭，否则 child 无法 receive EOF

        wait(0);
        exit(0);
    }
}
