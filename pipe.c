#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <wait.h>

int main(int argc, char *argv[])
{
    /* let's do "ls | grep foo". I have not checked the return values
     * of any of the system calls (pipe, close, dup2, execv) to keep
     * this example short. You should not ignore these. 
     */

    /* first, create the pipe, fds[0] is the read end of pipe, fds[1]
     * is the write end of pipe. 
     */
    int fds[2];
    pipe(fds);

    /* fork two children. remember that a return value of 0 means that
     * we are "in" the child. the parent is ignoring the return value
     * of both forks. 
     */
    if (fork() == 0) {
        close(fds[0]);                /* not reading from pipe */
        dup2(fds[1], STDOUT_FILENO);  /* this process' stdout should
                                         be the write end of the
                                         pipe. dup2 closes original
                                         stdout */
	close(fds[1]);                /* don't need this after dup2 */
        
        char *const args[] = { "/bin/ls", NULL };
        execv(args[0], args);
        _exit(EXIT_FAILURE);  /* on sucess, execv never returns */
    }
    
    if (fork() == 0) {
        close(fds[1]);               /* not writing to pipe */
        dup2(fds[0], STDIN_FILENO);  /* this process' stdin should be
                                        the read end of the pipe. dup2
                                        closes original stdin. */
	close(fds[0]);               /* don't need this after dup2 */

        char *const args[] = { "/s/std/bin/grep", "foo", NULL };
        execv(args[0], args);
        _exit(EXIT_FAILURE);
    }

    /* dup2 has signature: int dup2(int oldfd, int newfd); 
     * it closes newfd, and sets newfd to be oldfd (in the process'
     * open file table) in one atomic operation. 
     */

    /* the parent process only needed the pipe to give to its
     * children, it should NOT keep these open (can you figure out
     * why? what happens if the parent does not close them?). 
     */
    close(fds[0]);
    close(fds[1]);

    /* wait for the two children we forked */
    wait(NULL);
    wait(NULL);

    /* do you think it matters what order the children were forked? */

    return 0;
}
