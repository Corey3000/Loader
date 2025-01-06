#pragma once

namespace util
{
  struct ModuleStreamInformation
  {
    void* buffer;
    size_t buffer_size;
    size_t process_id;
    std::string pipe_name;
    std::string pipe_key;
    std::string pipe_message;
  };

  class Injector
  {
  public:
    Injector( ModuleStreamInformation msi );

    bool m_finished = false;
  private:
    void DefragmentMemory();
    void CreatePipeKey( std::string_view pipe_name, std::string_view pipe_key, size_t message_size );
  };

  inline std::optional<Injector> injector;
}