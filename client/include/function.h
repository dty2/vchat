#ifndef FUNCTION_H
#define FUNCTION_H

#include "common.h"
#include "package.h"
#include "connection.h"

namespace vchat {

class Function {
public:
  static Function* function;
  static void getinstance();
  Function(const Function&) = delete;
  Function& operator=(const Function&) = delete;

  // request service
  bool do_login(int, int);
  bool do_signin(int, int, std::string);
  void do_chat(int, int, std::string, int);
  std::string checkmessage(int);
  void do_addfriend(int);
  void do_deletefriend(int);

private:
  Function();
  ~Function();

};

} // namespace vchat

#endif // FUNCTION_H

