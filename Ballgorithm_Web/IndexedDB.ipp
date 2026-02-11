# pragma once

#if SIV3D_PLATFORM(WEB)
namespace s3d::Platform::Web
{
	namespace IndexedDB
	{
		namespace detail
		{
			void Mount(FilePathView path);

			AsyncTask<bool> SyncFSAsync(bool populate);
		}

		inline AsyncTask<bool> InitAsync(FilePathView path)
		{
			return InitAsync({ path });
		}
	
		inline AsyncTask<bool> InitAsync(std::initializer_list<FilePathView> paths)
		{
			for(const FilePathView path : paths) detail::Mount(path);
			return detail::SyncFSAsync(true);
		}

		inline bool Init(FilePathView path)
		{
			return Init({ path });
		}

		inline bool Init(std::initializer_list<FilePathView> paths)
		{
			auto task = InitAsync(std::move(paths));
			return s3d::Platform::Web::System::AwaitAsyncTask(task).value_or(false);
		}
		
		inline bool Save()
		{
			auto task = SaveAsync();
			return s3d::Platform::Web::System::AwaitAsyncTask(task).value_or(false);
		}

		inline AsyncTask<bool> SaveAsync()
		{
			return detail::SyncFSAsync(false);
		}
	}
}

#endif
