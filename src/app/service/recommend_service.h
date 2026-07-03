#pragma once

#include "src/app/client/book_client.h"
#include "src/app/model/book.h"
#include "src/app/recommend/book_vector_index.h"

#include <memory>
#include <mutex>
#include <optional>
#include <vector>

#define DEFAULT_SIMILAR_BOOK_LIMIT 5

class RecommendService {
public:
  RecommendService()
      : book_client_(nullptr)
  {
  }

  explicit RecommendService(std::shared_ptr<BookClient> book_client)
      : book_client_(std::move(book_client))
  {
    rebuild();
  }

  void set_book_client(std::shared_ptr<BookClient> book_client)
  {
    book_client_ = std::move(book_client);
    rebuild();
  }

  void rebuild()
  {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!book_client_) {
      index_.build({});
      return;
    }
    index_.build(book_client_->list_books());
  }

  std::optional<std::vector<Book>> similar_books(
      int book_id,
      int limit = DEFAULT_SIMILAR_BOOK_LIMIT) const
  {
    if (!book_client_ || book_id <= 0 || limit <= 0) {
      return std::nullopt;
    }

    auto source = book_client_->find_book(book_id);
    if (!source.has_value()) {
      return std::nullopt;
    }

    std::vector<int> candidate_ids;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!index_.ready()) {
        return std::vector<Book>{};
      }
      candidate_ids = index_.search_similar(*source, limit);
    }

    std::vector<Book> books;
    for (int candidate_id : candidate_ids) {
      auto book = book_client_->find_book(candidate_id);
      if (!book.has_value() || book->status != "active") {
        continue;
      }
      books.push_back(*book);
    }
    return books;
  }

private:
  std::shared_ptr<BookClient> book_client_;
  mutable std::mutex mutex_;
  BookVectorIndex index_;
};
