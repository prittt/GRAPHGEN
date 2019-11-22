// Copyright(c) 2016 - 2019
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met :
//
// *Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and / or other materials provided with the distribution.
//
// * Neither the name of GRAPHGEN nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#include "file_manager.h"

#include <algorithm>
#include <fstream>
#include <string>

#include <sys/stat.h>
#include <sys/types.h>

using namespace std;

const char filesystem::path::separator_ =
#ifdef GRAPHGEN_WINDOWS
'\\';
#else
'/';
#endif

bool filesystem::exists(const path& p)
{
    struct stat info;
    string s = p.string();
    const char* path_to_check = s.c_str();
    if (stat(path_to_check, &info) != 0) {
        //printf("cannot access %s\n", pathname);
        return false;
    }
    else if (info.st_mode & S_IFDIR) {
        //printf("%s is a directory\n", pathname);
        return true; // is directory
    }

    //printf("%s is no directory\n", pathname);
    return true; // is file
}

bool filesystem::exists(const path& p, error_code& ec)
{
    return filesystem::exists(p);
}

bool filesystem::create_directories(const path& p)
{
    string s(p.string());
    string parameters = "";
#if defined(GRAPHGEN_UNIX) || defined(GRAPHGEN_LINUX) || defined(GRAPHGEN_APPLE)
    // make it recursive by adding "-p" suffix
    parameters = "-p";
#endif

    if (!exists(path(s))) {
        if (system(("mkdir " + parameters + " \"" + s + "\"").c_str()) != 0) {
            //cerr << "Unable to find/create the output path " + s;
            return false;
        }
    }
    return true;
}

bool filesystem::create_directories(const path& p, error_code& ec)
{
    return filesystem::create_directories(p);
}

void filesystem::path::NormalizePath()
{
#if defined(GRAPHGEN_UNIX) || defined(GRAPHGEN_LINUX) || defined(GRAPHGEN_APPLE)
    std::replace(path_.begin(), path_.end(), '\\', '/');
#elif defined(GRAPHGEN_WINDOWS)
    std::replace(path_.begin(), path_.end(), '/', '\\');
#endif

    return;
}

void filesystem::copy(const path& from, const path& to)
{
    if (!filesystem::exists(from)) {
        return;
    }

    if (!filesystem::exists(to)) {
        if (filesystem::create_directories(to.parent_path())) {
            ifstream src(from.string());
            ofstream dst(to.string());

            dst << src.rdbuf();
        }
    }
}

void filesystem::copy(const path& from, const path& to, error_code& ec)
{
    filesystem::copy(from, to);
}