#ifndef USERINFO_HPP
#define USERINFO_HPP

#include "common.h"

typedef struct MessageInfo {
  int sender;
  int receiver;
  std::string msg;
  MessageInfo(){};
  MessageInfo(int sender_, int receiver_, std::string msg_) :
    sender(sender_),
    receiver(receiver_),
    msg(msg_) {}
}MessageInfo;

typedef struct FriendInfo {
  int friendid;
  FriendInfo(){};
  FriendInfo(int id_) : friendid(id_) {}
}FriendInfo;

typedef struct PersionalInfo {
  int id;
  int password;
  std::string username;
  PersionalInfo(){}
  PersionalInfo(int id_, int password_, std::string username_) :
    id(id_),
    password(password_),
    username(username_) {}
}PersionalInfo;

typedef struct UserInfo {
  PersionalInfo persionalinfo;
  std::list<FriendInfo> friendlist;
  std::list<MessageInfo> messagelist;
  UserInfo(){};
}UserInfo;

#endif // USERINFO_HPP