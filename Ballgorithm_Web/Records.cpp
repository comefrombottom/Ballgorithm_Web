# include "Records.hpp"
# include <emscripten/emscripten.h>

# include ".SECRET"

namespace s3d::detail {
	__attribute__((import_name("siv3dCreateXMLHTTPRequest")))
	extern int32 siv3dCreateXMLHTTPRequest();
	__attribute__((import_name("siv3dSetXMLHTTPRequestCallback")))
	extern void siv3dSetXMLHTTPRequestCallback(int32 id, void(*callback)(int32, void*), void* userData);
	__attribute__((import_name("siv3dSetXMLHTTPRequestErrorCallback")))
	extern void siv3dSetXMLHTTPRequestErrorCallback(int32 id, void(*callback)(int32, void*), void* userData);
	__attribute__((import_name("siv3dSetXMLHTTPRequestRequestHeader")))
	extern void siv3dSetXMLHTTPRequestRequestHeader(int32 id, const char* name, const char* value);
	__attribute__((import_name("siv3dGetXMLHTTPRequestResponseHeaders")))
	extern char* siv3dGetXMLHTTPRequestResponseHeaders(int32 id);
	//__attribute__((import_name("siv3dSendXMLHTTPRequest")))
	//extern void siv3dSendXMLHTTPRequest(int32 id, const char* data);
	__attribute__((import_name("siv3dSendXMLHTTPRequestWrapper")))
	extern void siv3dSendXMLHTTPRequestWrapper(int32 id, const char* data);
	__attribute__((import_name("siv3dOpenXMLHTTPRequest")))
	extern void siv3dOpenXMLHTTPRequestWithJSON(int32 id, const char* method, const char* url);
}

AsyncTask<HTTPResponse> SimplePostAsync(URLView url, const HashTable<String, String>& headers, const void* src, size_t size) {
	const auto wgetHandle = detail::siv3dCreateXMLHTTPRequest();

	s3d::detail::siv3dOpenXMLHTTPRequestWithJSON(wgetHandle, "POST", url.toUTF8().data());

	for (auto&& [key, value] : headers) {
		detail::siv3dSetXMLHTTPRequestRequestHeader(wgetHandle, key.toUTF8().data(), value.toUTF8().data());
	}

	constexpr auto CallBack = [](int32 requestID, void* userData) {
		char* responseHeader = detail::siv3dGetXMLHTTPRequestResponseHeaders(requestID);
		HTTPResponse response{ std::string(responseHeader) };
		::free(responseHeader);

		auto promise = static_cast<std::promise<HTTPResponse>*>(userData);
		promise->set_value(response);
		delete promise;

		EM_ASM("setTimeout(function() { _siv3dMaybeAwake(); }, 0)");
		};

	auto promise = new std::promise<HTTPResponse>;
	AsyncTask<HTTPResponse> task = promise->get_future();
	detail::siv3dSetXMLHTTPRequestCallback(wgetHandle, CallBack, promise);
	detail::siv3dSetXMLHTTPRequestErrorCallback(wgetHandle, CallBack, promise);

	if (src) {
		std::string body(static_cast<const char*>(src), size);
		s3d::detail::siv3dSendXMLHTTPRequestWrapper(wgetHandle, body.data());
	}
	else {
		s3d::detail::siv3dSendXMLHTTPRequestWrapper(wgetHandle, nullptr);
	}

	return task;
}

AsyncHTTPTask CreateGetTask() {
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	return SimpleHTTP::SaveAsync(Unicode::Widen(url), U"Bollagorithm/Sharing/leaderboard.json");
}

AsyncTask<HTTPResponse> CreatePostTask(String username, const Stage& stage) {
	if (!stage.m_isCleared) {
		return {};
	}
	
	const std::string url{ SIV3D_OBFUSCATE(LEADERBOARD_URL) };
	URL requestURL = U"{}?username={}&stagename={}"_fmt(Unicode::Widen(url), PercentEncode(username.length() == 0 ? U"?" : username), PercentEncode(stage.m_name));
	String recordPath = U"Ballagorithm/Sharing/{}.bin"_fmt(stage.m_name);

	auto snapshot = stage.createSnapshot();

	auto sc1 = snapshot.CalculateNumberOfObjects();
	auto sc2 = snapshot.CalculateTotalLength();

	HashTable<size_t, StageSnapshot> record;
	if (FileSystem::Exists(recordPath)) {
		Deserializer<BinaryReader> deserializer{ recordPath };
		deserializer(record);
	}

	record[sc1] = snapshot;

	{
		Serializer<BinaryWriter> serializer{ recordPath };
		serializer(record);
	}

	{
		Serializer<BinaryWriter> serializer{ recordPath };
	}

	Blob blob{ U"Ballagorithm/Stages/{}.bin"_fmt(stage.m_name) };
	auto blobStr = blob.base64Str();

	if (not RegExp(U"[a-zA-Z0-9?]+").fullMatch(username)) {
		username = U"?";
	}

	JSON json{};
	json[U"sc1"] = sc1;
	json[U"sc2"] = sc2;
	json[U"data"] = blobStr;
	const std::string secret{ SIV3D_OBFUSCATE(SECRET_KEY) };
	json[U"sig"] = MD5::FromText(String(U"{}{}{}{}{}{}"_fmt(username, stage.m_name, sc1, sc2, blobStr, Unicode::Widen(secret))).toUTF8()).asString();
	
	auto code = json.formatUTF8Minimum();

	return SimplePostAsync(requestURL, {}, code.data(), code.length() * sizeof(std::string::value_type));
}