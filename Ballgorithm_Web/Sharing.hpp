#pragma once

# include <Siv3D.hpp>
# include "Stage.hpp"

# include <emscripten/emscripten.h>

AsyncTask<HTTPResponse> SimplePostAsync(URLView url, const HashTable<String, String>& headers, const void* src, size_t size);

AsyncHTTPTask CreateGetTask();

void CreatePostTask(String username, const Stage& stage);