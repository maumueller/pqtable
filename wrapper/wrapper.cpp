#include <cstdint>
#include <cstdlib>
#include <cstdbool>

#include <algorithm>
#include <string>
#include <cstring>
#include <vector>
#include <sstream>
#include <fstream>
#include <iostream>

#include "pq_table.h"
#include "utils.h"

extern "C" {

static int M = 4;

bool configure(const char* var, const char* val) {
  if (strcmp(var, "M") == 0) {
    char* end;
    errno = 0;
    long k = strtol(val, &end, 10);
    if (errno != 0 || *val == 0 || *end != 0 || k < 0) {
      return false;
    } else {
      M = k;
      return true;
    }
  } else return false;
}

static std::vector<std::vector<float>> pointset;

bool end_configure(void) {
  return true;
}

static size_t entry_count = 0;

std::vector<float> parseEntry(const char* entry) {
  std::vector<float> e;
  std::string line(entry);
  float x;
  auto sstr = std::istringstream(line);
  while (sstr >> x) {
    e.push_back(x);
  }
  return e;
}

bool train(const char* entry) {
  auto parsed_entry = parseEntry(entry);
  pointset.push_back(parsed_entry);
  entry_count++;
  return true;
}

static pqtable::PQ* pq; 
static pqtable::PQTable* tbl;
static std::vector<std::pair<int,float>> results;
static std::vector<float> parsed_entry; 
static size_t position = 0;

void end_train(void) {
    // taken from examples/demo_siftsmall.cpp

    // Take first 5% of the point set as training data
    // Take at least 1024 points
    std::vector<std::vector<float>> learnset;
    std::cerr << "=== Learn Codes ===" << std::endl;
    for (size_t i = 0; i < std::max(pointset.size() / 20, 1024UL); i++) {
	learnset.push_back(pointset[i]);
    } 
    pq = new pqtable::PQ(pqtable::PQ::Learn(pointset, M));


    // (4) Encode vectors to PQ-codes
    std::cerr << "=== Encode Codes ===" << std::endl;
    pqtable::UcharVecs codes = pq->Encode(pointset);


    // (5) Build a PQTable
    std::cerr << "=== Build PQTable ===" << std::endl;
    tbl = new pqtable::PQTable(pq->GetCodewords(), codes);
    
    pointset.clear();
    pointset.shrink_to_fit();
    delete pq;
}

bool prepare_query(const char* entry) {
  position = 0;
  results.clear();
  parsed_entry = parseEntry(entry);
  return true;
}

size_t query(const char* entry, size_t k) {
  results = tbl->Query(parsed_entry, k); 
  return results.size();
}


size_t query_result(void) {
  if (position < results.size()) {
    auto index = results[position++].first;
    return index;
  } else return SIZE_MAX;
}

void end_query(void) {
  delete tbl;
}

}
