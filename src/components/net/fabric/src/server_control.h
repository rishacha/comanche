/*
   Copyright [2018] [IBM Corporation]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#ifndef _SERVER_CONTROL_H_
#define _SERVER_CONTROL_H_

#include <atomic>
#include <cstddef> /* uint16_t */
#include <map>
#include <memory> /* shared_ptr */
#include <mutex>
#include <queue>
#include <thread>
#include <tuple>

struct fid_fabric;
struct fi_info;
class Fabric_connection;
class Fd_control;
class Fd_socket;

class Pending_cnxns
{
public:
  using cnxn_t = std::shared_ptr<Fabric_connection>;
private:
  std::mutex _m; /* protects _q */
  using guard = std::unique_lock<std::mutex>;
  std::queue<cnxn_t> _q;
public:
  Pending_cnxns()
    : _m{}
    , _q{}
  {}
  void push(cnxn_t c)
  {
    guard g{_m};
    _q.push(c);
  }
  cnxn_t remove()
  {
    cnxn_t c;
    guard g{_m};
    if ( _q.size() != 0 )
    {
      c = _q.front();
      _q.pop();
    }
    return c;
  }
};
/*
 */

class Server_control
{
  using cnxn_t = std::shared_ptr<Fabric_connection>;
  Pending_cnxns _pending;

  using open_t = std::map<Fabric_connection *, cnxn_t>;
  open_t _open;

  int _end[2];
  int _pipe_errno;
  std::thread _th;

  static Fd_socket make_listener(std::uint16_t port);
  static void listen(Fd_socket &&listener, int end_fd, fid_fabric &fabric, const fi_info &info, Pending_cnxns &pend);
public:
  Server_control(fid_fabric &fabric, const fi_info &info, std::uint16_t port);
  ~Server_control();
  Fabric_connection * get_new_connection();
  std::vector<Fabric_connection *> connections();
  void close_connection(Fabric_connection * cnxn);
};

#endif
