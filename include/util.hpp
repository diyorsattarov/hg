#include <string>

const std::string kDbName = "database_2024";
const std::string kDbUser = "user";
const std::string kDbPassword = "password";
const std::string kDbHost = "172.19.0.2";
const int kDbPort = 5432;
const std::string kGetEndpoint = R"(/get/(\d+))";
const std::string kInsertEndpoint = R"(/insert/(\d+))";
const std::string kUpdateEndpoint = R"(/update/(\d+))";
const std::string kDeleteEndpoint = R"(/delete/(\d+))";

enum class Month {
  January = 1,
  February,
  March,
  April,
  May,
  June,
  July,
  August,
  September,
  October,
  November,
  December
};