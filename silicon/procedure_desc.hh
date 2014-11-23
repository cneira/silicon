#pragma once

namespace iod
{
  
  const char* type_string(const file*) { return "file"; }
  const char* type_string(const void*) { return "void"; }
  const char* type_string(const std::string*) { return "string"; }
  const char* type_string(const int*) { return "int"; }
  const char* type_string(const float*) { return "float"; }
  const char* type_string(const double*) { return "double"; }

  template <typename... T>
  std::string type_string(const sio<T...>* o)
  {
    std::stringstream res; res << "{";
    bool first = true;
    foreach(*o) | [&] (auto& m) {
      if (!first) res << ", ";
      first = false;
      res << m.symbol().name() << ": " << type_string(&m.value());
    };
    res << "}";
    return std::move(res.str());
  }
  
  struct procedure_desc {
    template <typename H>
    procedure_desc(H h)
      : name(h.name_)
    {
      // Get the argument list of the handle.
      typedef callable_return_type_t<typename H::content_type> ret_type;
      typedef callable_arguments_tuple_t<typename H::content_type> args_type;

      // Extract the sio argument.
      foreach(*(args_type*)0) | [&, this] (auto& a)
                       {
                         static_if<is_sio<decltype(a)>::value>(
                           [this] (auto& a) { this->fill_args(&a); },
                           [] (auto& a) { }, a);
                       };

      //typedef decltype(a) args2_type;
      //fill_args((args2_type*)0);

      return_type = type_string((ret_type*)0);
    }

    template <typename T>
    void fill_args(T*)
    {
      foreach(*(T*)0) | [&] (auto& a)
      {
        args.push_back(std::make_pair(a.symbol().name(), type_string(&a.value())));
      };
    }

    void fill_args(void*)
    {
    }
    
    std::string name;
    std::vector<std::pair<std::string, std::string>> args;
    std::string return_type;
  };

  template <typename S>
  std::vector<procedure_desc> server_api_description(const S& server)
  {
    auto handlers = server.get_handlers();
    std::vector<procedure_desc> res;
    for (int i = 0; i < handlers.size(); i++)
      res.push_back(handlers[i]->description());
    return std::move(res);
  }

  template <typename O>
  void print_procedure_desc(O& os, const procedure_desc& p)
  {
    os << p.name << "(";
    bool first = true;
    for (auto a : p.args)
    {
      if (!first) os << ", ";
      first = false;
      os << a.first << ": " << a.second;
    }
    os << ") -> " << p.return_type;
  }
  
  template <typename S>
  void print_server_api(const S& server)
  {
    auto desc = server_api_description(server);
    int i = 0;
    for (auto p : desc)
    {
      std::cout << "procedure " << i << ": " << p.name << "(";
      bool first = true;
      for (auto a : p.args)
      {
        if (!first) std::cout << ", ";
        first = false;
        std::cout << a.first << ": " << a.second;
      }
      std::cout << ") -> " << p.return_type << std::endl;;
      i++;
    }
  }
  
}