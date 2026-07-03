#pragma once

#include "src/app/model/book.h"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <string>
#include <vector>

#define BOOK_EMBED_DIM 64

inline void append_tokens(const std::string& text, std::vector<float>& embedding)
{
    std::string token;
    for (unsigned char ch : text) {
        if (std::isalnum(ch)) {
            token.push_back(static_cast<char>(std::tolower(ch)));
            continue;
        }
        if (token.empty()) {
            continue;
        }
        std::size_t hash = 2166136261u;
        for (char c : token) {
            hash ^= static_cast<unsigned char>(c);
            hash *= 16777619u;
        }
        embedding[hash % BOOK_EMBED_DIM] += 1.0f;
        token.clear();
    }
    if (!token.empty()) {
        std::size_t hash = 2166136261u;
        for (char c : token) {
            hash ^= static_cast<unsigned char>(c);
            hash *= 16777619u;
        }
        embedding[hash % BOOK_EMBED_DIM] += 1.0f;
    }
}

inline void normalize_embedding(std::vector<float>& embedding)
{
    float norm = 0.0f;
    for (float value : embedding) {
        norm += value * value;
    }
    if (norm <= 0.0f) {
        return;
    }
    norm = std::sqrt(norm);
    for (float& value : embedding) {
        value /= norm;
    }
}

inline std::vector<float> embed_book(const Book& book)
{
    std::vector<float> embedding(BOOK_EMBED_DIM, 0.0f);
    append_tokens(book.title, embedding);
    append_tokens(book.author, embedding);
    embedding[static_cast<std::size_t>(book.id) % BOOK_EMBED_DIM] += 0.25f;
    embedding[(BOOK_EMBED_DIM - 1)] = static_cast<float>(book.price_cents) / 100000.0f;
    normalize_embedding(embedding);
    return embedding;
}
