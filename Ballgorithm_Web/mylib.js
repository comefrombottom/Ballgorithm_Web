mergeInto(LibraryManager.library, {
    siv3dMountIDBFS: function (path) {
        const pathStr = UTF32ToString(path);
        FS.mount(IDBFS, {}, pathStr);
    },
    siv3dMountIDBFS__sig: "vi",
    siv3dMountIDBFS__deps: ["$FS"],

    siv3dSyncFSAsync: function (populate, promise) {
        FS.syncfs
            (
                populate,
                function (err) {
                    _siv3dSyncFSCallback(promise, !err);
                    _siv3dMaybeAwake();
                }
            );
    },
    siv3dSyncFSAsync__sig: "vii",
    siv3dSyncFSAsync__deps: ["$FS", "siv3dMaybeAwake", "siv3dSyncFSCallback"],
});