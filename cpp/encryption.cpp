#include <iostream>
#include <openssl/hmac.h>
#include <openssl/evp.h>
#include <vector>

std::vector<unsigned char> pseudoRandomFunction(const std::vector<unsigned char>& key, const std::vector<unsigned char>& data, size_t length) {
    std::vector<unsigned char> output(length);
    unsigned int len = length;

    HMAC_CTX* ctx = HMAC_CTX_new();
    HMAC_Init_ex(ctx, key.data(), key.size(), EVP_sha256(), NULL);
    HMAC_Update(ctx, data.data(), data.size());
    HMAC_Final(ctx, output.data(), &len);
    HMAC_CTX_free(ctx);

    return output;
}

int main() {
    std::vector<unsigned char> key = {'k', 'e', 'y'};
    std::vector<unsigned char> data = {'v', 'e', 'b', 'q'};
    size_t length = 32; // Output length

    std::vector<unsigned char> result = pseudoRandomFunction(key, data, length);

    std::cout << "PRF Output: ";
    for (unsigned char c : result) {
        printf("%02x", c);
    }
    std::cout << std::endl;

    return 0;
}