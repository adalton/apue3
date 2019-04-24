#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

struct foo {
	int a, b, c, d;
};

void
printfoo(const char *s, const struct foo *fp)
{
	printf("%s", s);
	printf("  structure at 0x%lx\n", (unsigned long)fp);
	printf("  foo.a = %d\n", fp->a);
	printf("  foo.b = %d\n", fp->b);
	printf("  foo.c = %d\n", fp->c);
	printf("  foo.d = %d\n", fp->d);
}

void *
thr_fn1(void *arg)
{
	struct foo* foo = malloc(sizeof(struct foo));

	if (foo != NULL) {
		foo->a = 1;
		foo->b = 2;
		foo->c = 3;
		foo->d = 4;

		printfoo("thread 1:\n", foo);
	}

	pthread_exit(foo);
}

void *
thr_fn2(void *arg)
{
	printf("thread 2: ID is %lu\n", (unsigned long)pthread_self());

	pthread_exit(NULL);
}

int
main(void)
{
	int err;
	pthread_t tid1;
	pthread_t tid2;
	struct foo *fp;

	err = pthread_create(&tid1, NULL, thr_fn1, NULL);
	if (err != 0) {
		perror("pthread_create");
		return err;
	}

	err = pthread_join(tid1, (void**) &fp);
	if (err != 0) {
		perror("pthread_join");
		return err;
	}

	sleep(1);

	printf("parent starting second thread\n");
	err = pthread_create(&tid2, NULL, thr_fn2, NULL);
	if (err != 0) {
		perror("pthread_create");
		return err;
	}

	sleep(1);
	printfoo("parent:\n", fp);

	free(fp);
	exit(0);
}
