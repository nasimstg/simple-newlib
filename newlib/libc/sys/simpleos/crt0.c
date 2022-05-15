extern void exit(int code);
extern int main(int argc, char* argv[]);
// extern int _init_signal(void);
extern char** environ;

void _start(int argc, char* argv[]) {
    // _init_signal();
    environ = argv + argc + 1;
    int ex = main(argc, argv);
    exit(ex);
}