
/*
 *  capsule - the game recording and overlay toolkit
 *  Copyright (C) 2017, Amos Wenger
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details:
 * https://github.com/itchio/capsule/blob/master/LICENSE
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 */

#pragma once

#include <lab/platform.h>
#include <lab/packet.h>

namespace capsule {

class Connection {
  public:
    Connection(std::string pipe_name);
    void Connect();
    void Close();

    void Write(const flatbuffers::FlatBufferBuilder &builder);
    char *Read();

    bool IsConnected() { return connected_; };
    std::string GetPipeName() { return pipe_name_; };
  
  private:
    std::string pipe_name_;
    std::string r_path_;
    std::string w_path_;
#if defined(LAB_WINDOWS)
    HANDLE pipe_r_ = INVALID_HANDLE_VALUE;
    HANDLE pipe_w_ = INVALID_HANDLE_VALUE;
#else // LAB_WINDOWS
    FILE *fifo_r_ = nullptr;
    FILE *fifo_w_ = nullptr;
#endif // !LAB_WINDOWS

    bool connected_ = false;
};

} // namespace capsule
