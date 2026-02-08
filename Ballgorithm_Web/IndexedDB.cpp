# include <emscripten.h>

# include "IndexedDB.hpp"

# if SIV3D_PLATFORM(WEB)
namespace s3d::Platform::Web::IndexedDB
{
	namespace detail
	{
		EM_JS
		(
			void, siv3dMountIDBFS, (const char32_t* path),
			{
				const pathStr = UTF32ToString(path);
				FS.mount(IDBFS, {}, pathStr);
			}
		);
		
		EM_JS
		(
			void, siv3dSyncFSAsync, (bool populate, std::promise<bool>* promise),
			{
				FS.syncfs
				(
					populate,
					function(err)
					{
						Module["_siv3dSyncFSCallback"](promise, !err);
						_siv3dMaybeAwake();
					}
				);
			}
		);

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
