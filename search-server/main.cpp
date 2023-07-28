#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "search_server.h"

using namespace std;



void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

void TestFindTopDocuments() {
  SearchServer search_server;
  search_server.AddDocument(0, "white cat and", DocumentStatus::ACTUAL, {8, -3});
  search_server.AddDocument(1, "fluffy cat fluffy tail", DocumentStatus::ACTUAL, {7, 2, 7});
  search_server.AddDocument(2, "groomed dog expressive eyes", DocumentStatus::BANNED, {5, -12, 2, 1});
  
    {
      const auto documents = search_server.FindTopDocuments("fluffy groomed cat");
      ASSERT_EQUAL(documents.size(),2);
    }
    
    {
         const auto documents = search_server.FindTopDocuments("fluffy groomed cat", DocumentStatus::BANNED);
          ASSERT_EQUAL(documents.size(),1);
    }
    
    {
         auto predicate = [](int document_id, DocumentStatus status [[maybe_unused]], int rating [[maybe_unused]]) {
            return document_id % 2 == 0;
          };
          const auto documents = search_server.FindTopDocuments("fluffy groomed cat", predicate);
          ASSERT_EQUAL(documents.size(),2); 
    }
 
}

void TestMatchDocument() {
  SearchServer search_server;
  search_server.AddDocument(0, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3});
  auto [words, status] = search_server.MatchDocument("white cat"s, 0);
  ASSERT_EQUAL(words.size(), 2);
  ASSERT_EQUAL(words[0],"cat");
  ASSERT_EQUAL(words[1], "white");
}

void TestStopWords() {
  SearchServer search_server;
  search_server.SetStopWords("and"s);
  search_server.AddDocument(0, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3});
  vector<Document> results = search_server.FindTopDocuments("and"s);
  ASSERT(results.empty());
}

void TestMinusWords() {
  SearchServer search_server;
  search_server.AddDocument(0, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3});
  vector<Document> results = search_server.FindTopDocuments("cat -white"s);
  ASSERT(results.empty());
}

void TestSortByRelevance() {
  SearchServer search_server;
  search_server.AddDocument(0, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3});
  search_server.AddDocument(1, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3, 10});
  vector<Document> results = search_server.FindTopDocuments("white cat"s);
  ASSERT_EQUAL(results.size(), 2);
  ASSERT_EQUAL(results[0].id , 1);
  ASSERT_EQUAL(results[1].id, 0);
}

void TestComputeAverageRating() {
  SearchServer search_server;
  search_server.AddDocument(0, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3, 10});
  vector<Document> results = search_server.FindTopDocuments("white cat"s);  
  ASSERT_EQUAL(results.at(0).rating, 5);
}

void TestPredicateFiltering() {
  SearchServer search_server;
  search_server.AddDocument(0, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3});
  search_server.AddDocument(1, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3, 10});
  vector<Document> results = search_server.FindTopDocuments(
      "white cat"s, [](int document_id, DocumentStatus , int ) {
        return document_id % 2 == 0;
      });
  ASSERT_EQUAL(results.size(), 1);
  ASSERT_EQUAL(results[0].id, 0);
}

void TestStatusFiltering() {
  SearchServer search_server;
  search_server.AddDocument(0, "white cat and"s,
                            DocumentStatus::ACTUAL, {8, -3});
  search_server.AddDocument(1, "white cat and"s,
                            DocumentStatus::BANNED, {8, -3, 10});
  vector<Document> results = search_server.FindTopDocuments(
      "white cat"s, DocumentStatus::ACTUAL);
  ASSERT_EQUAL(results.size(), 1);
  ASSERT_EQUAL(results[0].id, 0);
}

// Функция TestSearchServer является точкой входа для запуска тестов


// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
     RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
     RUN_TEST(TestFindTopDocuments);
     RUN_TEST(TestMatchDocument);
     RUN_TEST(TestStopWords);
     RUN_TEST(TestMinusWords);
     RUN_TEST(TestSortByRelevance);
     RUN_TEST(TestComputeAverageRating);
     RUN_TEST(TestPredicateFiltering);
     RUN_TEST(TestStatusFiltering);
}

// --------- Окончание модульных тестов поисковой системы -----------

int main() {
    TestSearchServer();
    // Если вы видите эту строку, значит все тесты прошли успешно
    cout << "Search server testing finished"s << endl;
}
