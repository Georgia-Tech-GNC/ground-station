#ifndef SQL_COMMANDS
#define SQL_COMMANDS

template <typename T, int N> struct Foo {
    T items[N];

    static constexpr int x = N;
};

void main() {
    const int n = 5;
    
    Foo<int, n> f;
}

#endif