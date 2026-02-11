#pragma once

# include <Siv3D.hpp>
# include "Stage.hpp"

class StageRecord
{
public:
	String m_stageName;
	StageSnapshot m_snapshot;

	size_t m_numberOfObjects;
	double m_totalLength;

	String m_author;
	String m_shareCode;

	MD5Value m_hash;

	StageRecord() = default;

	StageRecord(const Stage& stage, String author);

	bool isValid() const;

	AsyncTask<HTTPResponse> createPostTask();
	Optional<String> processPostTask(const AsyncTask<HTTPResponse>& task);
	
	static AsyncTask<HTTPResponse> createGetTask(String shareCode);
	static AsyncTask<HTTPResponse> createGetLeaderboradTask(String stageName);

	static Optional<StageRecord> processGetTask(const AsyncTask<HTTPResponse>& task);
	static Optional<Array<StageRecord>> processGetLeaderboardTask(const AsyncTask<HTTPResponse>& task);
}

/*
AsyncTask<HTTPResponse> SimplePostAsync(URLView url, const HashTable<String, String>& headers, const void* src, size_t size);

AsyncHTTPTask CreateGetTask();

AsyncTask<HTTPResponse> CreatePostTask(String username, const Stage& stage);
*/