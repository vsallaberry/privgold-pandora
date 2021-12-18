#import <Carbon/Carbon.h>

int main(int argc, const char *argv[]) {
    unsigned char keyMap[sizeof(KeyMap)];

    memset(keyMap, 0, sizeof(keyMap));
    GetKeys((BigEndianUInt32 *) keyMap);
    for (int i = 0; i < sizeof(keyMap) / sizeof(*keyMap); ++i) {
        if (keyMap[i] != 0) {
            printf("1\n");
            return 0;
        }
    }
    printf("0\n");
    return 0;
}
