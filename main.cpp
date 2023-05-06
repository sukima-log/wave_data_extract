#include <iostream>
#include <stdio.h>
#include <string.h>

#define PATH 255                           // ファイルパスの最大長
#define FILE_NAME "./wav/adventurers.WAV"  // 入力ファイルパス

// 各チャンク先頭の「tag」と「size」を表す構造体
typedef struct _chunk {
    char            id[4];  // tag 4文字
    unsigned int    size;   // Chunk size
} ChunkHead;

/* ------ Riff Chunk ------ */
// 「tag="R","I","F","F"」「size=4」
typedef struct _riffChunk {
    ChunkHead   head;       // 共通Chunkヘッダー
    char        format[4];  // 「format="W""A""V""E"」
} RiffChunk;

// Wave Format Chunk 固定基本情報
// size以降6つの要素は長さ固定、それ以降は読み込んだsizeに従う
typedef struct _wavFmt {
    unsigned short  audioFormat;    // Wavフォーマット
    unsigned short  channels;       // チャンネル数
    unsigned int    samplePerSecond;// サンプリング周波数
    unsigned int    bytesPerSecond; // データ量/秒
    unsigned short  blockAlign;     // 単位バイト幅
    unsigned short  bitsPerSample;  // 量子化ビット数
} WaveFileFormat;

/* ------ Wave Format Chunk ------ */
// 「tag="f","m","t"," "」「size=不定」
typedef struct _wavFmtChunk {
    ChunkHead       chunk;  // 共通Chunkヘッダー
    WaveFileFormat  format; // 楽曲データ 固定基本情報
} WaveFormatChunk;

/* ------ Wave Data Chunk ------ */
// 「tag="d","a","t","a"」「size=不定」
// 「Header＋データ内容」をメモリ領域に確保


int main (void) {

    /* ファイル入力 */
    char filename_in[PATH];    // 入力ファイル名
    FILE *file = NULL;
    sprintf(filename_in, FILE_NAME);
    file = fopen(filename_in, "rb");
    // ファイルオープン失敗チェック
    if (file == NULL) {
        printf("※ファイルオープン失敗\n");
        fclose(file);
        return 1;
    }

    /* 音楽データ読み込み */
    size_t read_size;
    // RIFF Chunk読み込み（ヘッダーチェック）
    RiffChunk riff;
    read_size = fread(&riff, sizeof(RiffChunk), 1, file);
    if (read_size != 1) return 1;

    if (strncmp(riff.head.id, "RIFF", 4) != 0) {
        fclose(file);
        printf("※ファイルフォーマットの異常\n");
        return 1;
    }
    if (strncmp(riff.format, "WAVE", 4) != 0) {
        fclose(file);
        printf("※ファイルフォーマットの異常\n");
        return 1;
    }

    ChunkHead chunk;
    WaveFileFormat format;

    unsigned long cu_pos = ftell(file); // ファイル現在地
    // ここでは量子化ビット数16bitのものを扱う
    if (sizeof(short) != 2) printf("buffer_16のデータ型を2byteのものに書き換え\n");
    short *buffer_16;

    // ファイル内全検索（終点到達まで）
    while (cu_pos < riff.head.size + sizeof(ChunkHead)) {
        read_size = fread(&chunk, sizeof(ChunkHead), 1, file);
        if (read_size != 1) return 1;

        // フォーマットエラー検知
        if (chunk.size < 0) break;
        // fmt Chunk読み込み
        if (strncmp(chunk.id, "fmt ", 4) == 0) {
            read_size = fread(&format, std::min(chunk.size, (unsigned int)sizeof(WaveFileFormat)), 1, file);
            if (read_size != 1) return 1;
            fseek(file, chunk.size - (unsigned int)sizeof(WaveFileFormat), SEEK_CUR);
        }
        // data Chunk読み込み
        else if (strncmp(chunk.id, "data", 4) == 0) {
            if (format.bitsPerSample == 16) {
                // データ格納配列動的確保 (chunk.size/2 : バイトを16bit型で取得するため)
                buffer_16 = (short *)calloc(chunk.size/2, sizeof(short));
                read_size = fread(buffer_16, sizeof(short), chunk.size/2, file);
                if (read_size != chunk.size/2) return 1;
            } else {
                fclose(file);
                printf("量子化ビット数 16bit以外\n");
                return 1;
            }
        }
        // 認識不可ChunkをSkip
        else {
            // 認識できないChunkのSkip
            fseek(file, chunk.size, SEEK_CUR);
        }
        // 現在のファイル位置取得
        cu_pos = ftell(file);
    }

    // ファイルを閉じる
    fclose(file);

    /* 確認 */
    printf("サンプリング周波数 : %d Hz\n", format.samplePerSecond);
    printf("量子化ビット数 : %d bit\n", format.bitsPerSample);
    printf("ステレオ「2」/ モノラル「1」 : %d\n", format.channels);

    return 0;
}