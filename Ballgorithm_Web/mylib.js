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

    siv3dSendXMLHTTPRequestWrapper: function (id, dataPtr) {
        runtimeKeepalivePush();

        siv3dXMLHTTPRequestList[id].addEventListener("load", function() {
            runtimeKeepalivePop();
        });
        siv3dXMLHTTPRequestList[id].addEventListener("error", function() {
            runtimeKeepalivePop();
        });

        const data = dataPtr ? UTF8ToString(dataPtr) : null;
        siv3dXMLHTTPRequestList[id].send(data);
    },
    siv3dSendXMLHTTPRequestWrapper__sig: "vii",
    siv3dSendXMLHTTPRequestWrapper__deps: ["$siv3dSendXMLHTTPRequest"],

    siv3dOpenXMLHTTPRequestWithJSON: function (id, methodPtr, urlPtr) {
        const http = siv3dXMLHTTPRequestList[id];
        const method = UTF8ToString(methodPtr);
        const url = UTF8ToString(urlPtr);

        http.open(method, url, true);
        http.responseType = "arraybuffer";

        if (method == "POST") {
            http.setRequestHeader("Content-type", "text/plain");
        }
    },
    siv3dOpenXMLHTTPRequestWithJSON__sig: "viii",
    siv3dOpenXMLHTTPRequestWithJSON__deps: ["$siv3dXMLHTTPRequestList"],
});