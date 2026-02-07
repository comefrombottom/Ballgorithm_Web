# pragma once

# include <Siv3D/AsyncTask.hpp>
# include <Siv3D/FileSystem.hpp>
# include <Siv3D/System.hpp>

namespace s3d::Platform::Web
{
	/// @brief ファイルシステム内のデータをIndexedDB上に保存するための関数群を提供します。
	namespace IndexedDB
	{
		/// @brief 指定されたパス以下をIndexedDBの同期対象に設定し、既にデータが存在する場合には非同期でロードします。
		inline AsyncTask<bool> InitAsync(FilePathView path);

		/// @brief 指定されたパス以下をIndexedDBの同期対象に設定し、既にデータが存在する場合にはロードします。
		inline bool Init(FilePathView path);

		/// @brief 指定されたパス以下をIndexedDBの同期対象に設定し、既にデータが存在する場合には非同期でロードします。
		inline AsyncTask<bool> InitAsync(std::initializer_list<FilePathView> path);

		/// @brief 指定されたパス以下をIndexedDBの同期対象に設定し、既にデータが存在する場合にはロードします。
		inline bool Init(std::initializer_list<FilePathView> paths);

		/// @brief 現在のファイルの内容をIndexedDBに非同期で保存します。
		inline AsyncTask<bool> SaveAsync();

		/// @brief 現在のファイルの内容をIndexedDBに保存します。
		inline bool Save();
	}
}

# include "IndexedDB.ipp"