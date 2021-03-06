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

#ifndef _FABRIC_HELP_H_
#define _FABRIC_HELP_H_

/* 
 * Authors: 
 * 
 */

#include <rdma/fi_errno.h> /* FI_AGAIN */
#include "fabric_ptr.h" /* fid_unique_ptr */

#include <unistd.h> /* ssize_t */
#include <cstdint>
#include <map>
#include <memory> /* shared_ptr */
#include <string>

struct fi_av_attr;
struct fi_cq_attr;
struct fi_info;
struct fi_eq_attr;
struct fi_fabric_attr;

struct fid_cq;
struct fid_domain;
struct fid_fabric;
struct fid_ep;
struct fid_eq;
struct fid_mr;
struct fid_pep;

/** 
 * Fabric/RDMA-based network component
 * 
 */

std::shared_ptr<fid_domain> make_fid_domain(fid_fabric &fabric, fi_info &info, void *context);

std::shared_ptr<fid_fabric> make_fid_fabric(fi_fabric_attr &attr, void *context);

std::shared_ptr<fi_info> make_fi_info(int version, const char *node, const char *service, const fi_info *hints);

std::shared_ptr<fi_info> make_fi_info(const std::string &, std::uint64_t caps, fi_info &hints);

std::shared_ptr<fi_info> make_fi_info(fi_info &hints);

std::shared_ptr<fi_info> make_fi_fabric_spec(const std::string& json_configuration);

std::shared_ptr<fid_ep> make_fid_aep(fid_domain &domain, fi_info &info, void *context);

std::shared_ptr<fid_pep> make_fid_pep(fid_fabric &fabric, fi_info &info, void *context);

std::shared_ptr<fid_eq> make_fid_eq(fid_fabric &fabric, fi_eq_attr *attr, void *context);

std::shared_ptr<fi_info> make_fi_info();

std::shared_ptr<fi_info> make_fi_infodup(const fi_info &info_, const std::string &why_);

fid_unique_ptr<fid_mr> make_fid_mr_reg(
  fid_domain &domain, const void *buf, size_t len,
  uint64_t access, uint64_t requested_key,
  uint64_t flags);

fid_unique_ptr<fid_cq> make_fid_cq(fid_domain &domain, fi_cq_attr &attr, void *context);
fid_unique_ptr<fid_av> make_fid_av(fid_domain &domain, fi_av_attr &attr, void *context);

/* fi_fabric, fi_close (when called on a fabric) and most fi_poll functions FI_SUCCESS; others return 0 */
void (check_ge_zero)(int r, const char *file, unsigned line);

#define CHECKZ(V) (check_ge_zero)((V), __FILE__, __LINE__)

[[noreturn]] void not_expected(const std::string &who);
[[noreturn]] void not_implemented(const std::string &who);

template <class ... Args>
  void post(ssize_t (*post_fn)(Args ... args), void (*comp_fn)(void *, std::uint64_t), uint64_t seq, void *ctx, const char *op, Args &&... args)
  {
    auto r = post_fn(args ...);
    while ( r == -FI_EAGAIN )
    {
      comp_fn(ctx, seq);
      ++seq;
      r = post_fn(args ...);
    }
    (check_ge_zero)(r, op, 0);
  }

#endif
