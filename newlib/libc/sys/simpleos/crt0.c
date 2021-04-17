extern void exit(int code);
extern int main(int argc, char* argv[]);
// extern int _init_signal(void);

void _start(int argc, char* argv[]) {
    // _init_signal();
    int ex = main(argc, argv);
    exit(ex);
}