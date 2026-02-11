# if SIV3D_PLATFORM(WEB)
# include <emscripten.h>

# include "IndexedDB.hpp"

namespace s3d::Platform::Web::IndexedDB
{
	namespace detail
	{
		__attribute__((import_name("siv3dMountIDBFS")))
		void siv3dMountIDBFS(const char32* path);

		__attribute__((import_name("siv3dSyncFSAsync")))
		void siv3dSyncFSAsync(bool populate, std::promise<bool>* promise);

		__attribute__((used, export_name("siv3dSyncFSCallback")))
		void siv3dSyncFSCallback(std::promise<bool>* promise, bool result)
		{
			promise->set_value(result);
			delete promise;
		}

		void Mount(FilePathView path)
		{
			FileSystem::CreateDirectories(path);
			siv3dMountIDBFS(path.data());
		}

		AsyncTask<bool> SyncFSAsync(bool populate)
		{
			auto promise = new std::promise<bool>();
			siv3dSyncFSAsync(populate, promise);
			return promise->get_future();
		}
	}
}
# endif
