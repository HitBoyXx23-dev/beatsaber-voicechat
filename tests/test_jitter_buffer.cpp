#include <cstdio>
#include "../src/jitter_buffer.hpp"

using namespace voicechat;

static int failures = 0;
#define CHECK(cond) do { if (!(cond)) { std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond); failures++; } } while (0)

int main() {
    // Out-of-order packets come back in index order.
    {
        JitterBuffer jb(3);
        jb.Push(2, {2});
        jb.Push(0, {0});
        CHECK(!jb.Pop().has_value());  // not yet full
        jb.Push(1, {1});
        auto a = jb.Pop(); CHECK(a && (*a)[0] == 0);
        jb.Push(3, {3});
        auto b = jb.Pop(); CHECK(b && (*b)[0] == 1);
        CHECK(!jb.Pop().has_value());  // only 2 left, waits for depth
        jb.Push(4, {4});
        auto c = jb.Pop(); CHECK(c && (*c)[0] == 2);
    }
    // A packet that arrives after its slot has been released is dropped.
    {
        JitterBuffer jb(1);
        jb.Push(5, {5});
        CHECK(jb.Pop().has_value());
        jb.Push(4, {4});  // late
        CHECK(jb.Size() == 0);
    }
    // A missing packet does not stall playback.
    {
        JitterBuffer jb(2);
        jb.Push(0, {0});
        jb.Push(2, {2});
        auto a = jb.Pop(); CHECK(a && (*a)[0] == 0);
        auto b = jb.Pop(); CHECK(!b.has_value());  // only one left, waits for depth
        jb.Push(3, {3});
        auto c = jb.Pop(); CHECK(c && (*c)[0] == 2);
    }
    // Reset clears state.
    {
        JitterBuffer jb(1);
        jb.Push(7, {7});
        jb.Pop();
        jb.Reset();
        jb.Push(0, {0});
        CHECK(jb.Pop().has_value());
    }
    if (failures == 0) std::printf("All jitter buffer tests passed.\n");
    return failures == 0 ? 0 : 1;
}
