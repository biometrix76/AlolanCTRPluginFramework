#ifndef ULTRASUN_LIVE_CAUGHT_HPP
#define ULTRASUN_LIVE_CAUGHT_HPP
#include <string>
namespace CTRPluginFramework { namespace LiveCaught {
void Tick(bool entering);
void Rescan();
void SelectImage(const std::string &directory, const std::string &image);
void Draw();
}}
#endif
