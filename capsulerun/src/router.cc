
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

#include "router.h"

#include <lab/packet.h>

#include "capsule/messages_generated.h"
#include "logging.h"

#include <thread>
#include <sstream>

namespace capsule {

Router::~Router() {
  // muffin
}

void Router::Start() {
  new std::thread(&Router::Run, this);
}

void Router::Run() {
  while (true) {
    auto conn = new Connection(pipe_path_);

    Log("Router: Accepting connection...");
    conn->Connect();

    if (!conn->IsConnected()) {
      Log("Router: Failed to accept connection, bailing out");
      exit(127);
    }

    had_connections_ = true;

    std::ostringstream oss;    
    oss << "capsule" << seed_++;
    auto new_conn_name = oss.str();

    Log("Router: dispatching connection to %s", new_conn_name.c_str());
    loop_->AddConnection(new Connection(new_conn_name));

    {
      flatbuffers::FlatBufferBuilder builder(32);

      auto pipe = builder.CreateString(new_conn_name);
      auto hkp = messages::CreateReadyForYou(builder, pipe);
      auto pkt = messages::CreatePacket(
          builder,
          messages::Message_ReadyForYou,
          hkp.Union()
      );

      builder.Finish(pkt);
      Log("Router: sending ReadyForYou");
      conn->Write(builder);
    }

    // Sleep(1000);

    conn->Close();
    delete conn;
  }
}

} // namespace capsule
