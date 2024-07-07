#include "vchat.h"
#include "info.h"

namespace vchat {

Chat::Chat(int id_, Function& function_, ScreenInteractive& screen_)
  : id(id_), function(function_), screen(screen_) {
  getmessagelist();
  getinputarea();
  auto cchatpage = Container::Vertical({list, inputarea}, &inputfocus);
  auto echatpage = CatchEvent(cchatpage, [&](Event event){
    if(event == Event::CtrlN) {
      list->TakeFocus();
      if(selected_msg != messagelist.size() - 1)
        selected_msg ++;
      return true;
    }
    else if(event == Event::CtrlP) {
      list->TakeFocus();
      if(selected_msg)
        selected_msg --;
      return true;
    }
    else {
      inputarea->TakeFocus();
    }
    return false;
  });
  this->content = echatpage | flex;
}

std::vector<MessageInfo> Chat::getmessage(int id) {
  std::vector<MessageInfo> temp;
  for (auto& v : Info::info->userinfo.messagelist) {
    if (v.receiver == id || v.sender == id)
      temp.emplace_back(v);
  }
  return temp;
}

void Chat::updatemessagelist() {
  messagelist.clear();
  std::vector<MessageInfo> messages = getmessage(this->id);
  for(auto& value : messages) {
    messagelist.emplace_back(
      Renderer([=](bool focused){
        Element element;
        if(value.sender == id) {
          element = hbox(text(value.msg) | border, filler());
        } else if(value.receiver == id){
          element = hbox(filler(), text(value.msg) | border);
        }
        if(focused) element |= focus;
        return element;
      })
    );
  }
}

void Chat::getmessagelist() {
  //updatemessagelist();
  auto rlist = Renderer([=]{
    messagelist.clear();
    std::vector<MessageInfo> messages = getmessage(this->id);
    for(auto& value : messages) {
      messagelist.emplace_back(
        Renderer([=](bool focused){
          Element element;
          if(value.sender == id) {
            element = hbox(text(value.msg) | border, filler());
          } else if(value.receiver == id){
            element = hbox(filler(), text(value.msg) | border);
          }
          if(focused) element |= focus;
          return element;
        })
      );
    }
    auto show = Container::Vertical(messagelist, &selected_msg) | vscroll_indicator | frame;
    Elements container;
    for(auto& value : messagelist) {
      container.emplace_back(value->Render());
    }
    return show->Render();
  });
  auto elist = CatchEvent(rlist, [&](Event event){
    if(event == Event::CtrlN) {
      if(selected_msg != list->ChildCount() - 1) selected_msg ++;
      return true;
    }
    else if(event == Event::CtrlP) {
      if(selected_msg) selected_msg --;
      return true;
    }
    else if(event == Event::Special("ssendmsg")) {
      LOG(INFO) << "get ssendmsg";
      return true;
    }
    return false;
  });
  this->list = elist | border | flex;
}

void Chat::getinputarea() {
  auto cinput = Input(&input, "Here to input...");
  auto einput = CatchEvent(cinput, [&](Event event){
    if(event == Event::CtrlY) {
      std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      std::tm* now_tm = std::localtime(&now);
      std::ostringstream oss;
      oss << std::setfill('0') << std::setw(2) << std::to_string(now_tm->tm_year - 100) // year
          << std::setfill('0') << std::setw(2) << std::to_string(now_tm->tm_mon + 1) // month
          << std::setfill('0') << std::setw(2) << std::to_string(now_tm->tm_mday) // date
          << std::setfill('0') << std::setw(2) << std::to_string(now_tm->tm_hour) // hour
          << std::setfill('0') << std::setw(2) << std::to_string(now_tm->tm_min) // minute
          << std::setfill('0') << std::setw(2) << std::to_string(now_tm->tm_sec); // second
      if(!input.empty())
        if(!function.sendmsg(id, Info::info->userinfo.persionalinfo.id, input,
          static_cast<int64_t>(std::stoll(oss.str())))) {
          input.clear();
          //updatemessagelist();
          screen.PostEvent(Event::Special("ssendmsg"));
          list->TakeFocus();
        }
      return true;
    } else return false;
  }) | size(HEIGHT, EQUAL, 2);
  this->inputarea = einput | border;
}

Friend::Friend(int id_, Function& function_)
  : id(id_), function(function_) {
  content = Container::Horizontal({
    Button(" 󰍡  发消息 ", [&]{ }, ButtonOption::Ascii()),
    Button(" 󰆴 删除好友 ", [&]{ }, ButtonOption::Ascii()),
  });
}

Myself::Myself(Function& function_)
  : function(function_) {
  content = Button( "myself", [&] { }, ButtonOption::Ascii());
}

Vchat::Vchat(int& now_, Function& function_, ScreenInteractive& screen_)
  : now(now_), function(function_), screen(screen_) {
  LOG(INFO) << "friendlist size: " << Info::info->userinfo.friendlist.size();
  for(auto& v : Info::info->userinfo.friendlist)
    this->chats[v.friendid] = new Chat(v.friendid, function, screen);
  LOG(INFO) << "chats size: " << chats.size();
  for(auto& v : Info::info->userinfo.friendlist)
    this->friends[v.friendid] = new Friend(v.friendid, function);
  this->myself = new Myself(function);
  this->getpage();
  this->getcatalogue();
  // main
  auto cmain = Container::Tab({catalogue, pages}, &catalogue_toggle);
  auto rmain = Renderer(cmain, [=] {
    Element show = pages->Render();
    if (!catalogue_toggle) {
      show = hbox(catalogue->Render(), show);
      catalogue->TakeFocus();
    }
    return show;
  });
  auto emain = CatchEvent(rmain, [&](Event event) {
    if (event == Event::CtrlL) {
      catalogue_toggle = catalogue_toggle ? 0 : 1;
      return true;
    }
    return false;
  });
  content = emain;
}

void Vchat::getpage() {

  auto page = Container::Tab({
    chats.begin()->second->content,
    friends.begin()->second->content,
    myself->content
  }, &option_selected);
  this->pages = page;
}

void Vchat::getcatalogue() {
  // option
  auto option = Container::Horizontal({
    Button( "   消息 ", [&] { page_selected = CHAT_PAGE; }, ButtonOption::Ascii()),
    Button( "   好友 ", [&] { page_selected = FRIENDS_PAGE; }, ButtonOption::Ascii()),
    Button( "   自己 ", [&] { page_selected = MYSELF_PAGE; }, ButtonOption::Ascii())},
  &option_selected);
  // message list
  auto messageslist = Container::Vertical( {
    Button( " messagelist1", [&] {}, ButtonOption::Ascii()),
    Button( " messagelist2", [&] {}, ButtonOption::Ascii()),
    Button( " messagelist3", [&] {}, ButtonOption::Ascii()),
  },
  &messages_selected);
  // friend list
  auto friendslist = Container::Vertical( {
    Button( " friendlist1", [&] {}, ButtonOption::Ascii()),
    Button( " friendlist2", [&] {}, ButtonOption::Ascii()),
    Button( " friendlist3", [&] {}, ButtonOption::Ascii()),
  },
  &friends_selected);
  // myself list
  auto myselflist = Container::Vertical( {
    Button( " myselflist1", [&] {}, ButtonOption::Ascii()),
    Button( " myselflist2", [&] {}, ButtonOption::Ascii()),
    Button( " myselflist3", [&] {}, ButtonOption::Ascii()),
  },
  &myself_selected);
  // list
  auto list = Container::Tab({messageslist, friendslist, myselflist}, &option_selected);
  // main
  auto catalogue = Container::Vertical({option, list});
  auto ecatalogue = CatchEvent(catalogue, [=](Event event) {
    if (event == Event::CtrlP) {
      if (!option->Focused())
        switch (option_selected) {
          case 0: if (messages_selected) messages_selected--; break;
          case 1: if (friends_selected) friends_selected--; break;
          case 2: if (myself_selected) myself_selected--; break;
        }
      list->TakeFocus();
    } else if (event == Event::CtrlN) {
      if (!option->Focused())
        switch (option_selected) {
        case 0: if (messages_selected != 2) messages_selected++; break;
        case 1: if (friends_selected != 2) friends_selected++; break;
        case 2: if (myself_selected != 2) myself_selected++; break;
        }
      list->TakeFocus();
    } else if (event == Event::CtrlB) {
      if (!list->Focused())
        if (!list_selected && option_selected)
          option_selected--;
      option->TakeFocus();
    } else if (event == Event::CtrlF) {
      if (!list->Focused())
        if (!list_selected && option_selected != 2)
          option_selected++;
      option->TakeFocus();
    } else if (event == Event::Return) {
      return false;
    }
    return true;
  });
  this->catalogue = ecatalogue | border;
}

} // namespace vchat
