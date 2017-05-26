// Copyright (c) 2010 The WebM project authors. All Rights Reserved.
//
// Use of this source code is governed by a BSD-style license
// that can be found in the LICENSE file in the root of the source
// tree. An additional intellectual property rights grant can be found
// in the file PATENTS.  All contributing project authors may
// be found in the AUTHORS file in the root of the source tree.

#ifndef MKVPARSER_HPP
#define MKVPARSER_HPP

#include <cstdlib>
#include <cstdio>
#include <cstddef>

namespace mkvparser
{

const int E_FILE_FORMAT_INVALID = -2;
const int E_BUFFER_NOT_FULL = -3;

class IMkvReader
{
public:
    virtual int Read(long long pos, long len, unsigned char* buf) = 0;
    virtual int Length(long long* total, long long* available) = 0;
protected:
    virtual ~IMkvReader();
};

long long GetUIntLength(IMkvReader*, long long, long&);
long long ReadUInt(IMkvReader*, long long, long&);
long long SyncReadUInt(IMkvReader*, long long pos, long long stop, long&);
long long UnserializeUInt(IMkvReader*, long long pos, long long size);
float Unserialize4Float(IMkvReader*, long long);
double Unserialize8Double(IMkvReader*, long long);
short Unserialize2SInt(IMkvReader*, long long);
signed char Unserialize1SInt(IMkvReader*, long long);
bool Match(IMkvReader*, long long&, unsigned long, long long&);
bool Match(IMkvReader*, long long&, unsigned long, char*&);
bool Match(IMkvReader*, long long&, unsigned long, unsigned char*&, size_t&);
bool Match(IMkvReader*, long long&, unsigned long, double&);
bool Match(IMkvReader*, long long&, unsigned long, short&);

void GetVersion(int& major, int& minor, int& build, int& revision);

struct EBMLHeader
{
    EBMLHeader();
    ~EBMLHeader();
    long long m_version;
    long long m_readVersion;
    long long m_maxIdLength;
    long long m_maxSizeLength;
    char* m_docType;
    long long m_docTypeVersion;
    long long m_docTypeReadVersion;

    long long Parse(IMkvReader*, long long&);
    void Init();
};


class Segment;
class Track;
class Cluster;

class Block
{
    Block(const Block&);
    Block& operator=(const Block&);

public:
    const long long m_start;
    const long long m_size;

    Block(long long start, long long size, IMkvReader*);
    ~Block();

    long long GetTrackNumber() const;
    long long GetTimeCode(const Cluster*) const;  //absolute, but not scaled
    long long GetTime(const Cluster*) const;      //absolute, and scaled (ns)
    bool IsKey() const;
    void SetKey(bool);
    bool IsInvisible() const;

    int GetFrameCount() const;  //to index frames: [0, count)

    struct Frame
    {
        long long pos;  //absolute offset
        long len;

        long Read(IMkvReader*, unsigned char*) const;
    };

    const Frame& GetFrame(int frame_index) const;

private:
    long long m_track;   //Track::Number()
    short m_timecode;  //relative to cluster
    unsigned char m_flags;

    Frame* m_frames;
    int m_frame_count;

};


class BlockEntry
{
    BlockEntry(const BlockEntry&);
    BlockEntry& operator=(const BlockEntry&);

public:
    virtual ~BlockEntry();
    virtual bool EOS() const = 0;
    virtual const Cluster* GetCluster() const = 0;
    virtual size_t GetIndex() const = 0;
    virtual const Block* GetBlock() const = 0;
    //virtual bool IsBFrame() const = 0;

protected:
    BlockEntry();

};


class SimpleBlock : public BlockEntry
{
    SimpleBlock(const SimpleBlock&);
    SimpleBlock& operator=(const SimpleBlock&);

public:
    SimpleBlock(Cluster*, size_t, long long start, long long size);

    bool EOS() const;
    const Cluster* GetCluster() const;
    size_t GetIndex() const;
    const Block* GetBlock() const;
    //bool IsBFrame() const;

protected:
    Cluster* const m_pCluster;
    const size_t m_index;
    Block m_block;

};


class BlockGroup : public BlockEntry
{
    BlockGroup(const BlockGroup&);
    BlockGroup& operator=(const BlockGroup&);

public:
    BlockGroup(Cluster*, size_t, long long, long long);
    ~BlockGroup();

    bool EOS() const;
    const Cluster* GetCluster() const;
    size_t GetIndex() const;
    const Block* GetBlock() const;
    //bool IsBFrame() const;

    short GetPrevTimeCode() const;  //relative to block's time
    short GetNextTimeCode() const;  //as above

protected:
    Cluster* const m_pCluster;
    const size_t m_index;

private:
    BlockGroup(Cluster*, size_t, unsigned long);
    void ParseBlock(long long start, long long size);

    short m_prevTimeCode;
    short m_nextTimeCode;

    //TODO: the Matroska spec says you can have multiple blocks within the
    //same block group, with blocks ranked by priority (the flag bits).
    //For now we just cache a single block.
#if 0
    typedef std::deque<Block*> blocks_t;
    blocks_t m_blocks;  //In practice should contain only a single element.
#else
    Block* m_pBlock;
#endif

};


class Track
{
    Track(const Track&);
    Track& operator=(const Track&);

public:
    Segment* const m_pSegment;
    const long long m_element_start;
    const long long m_element_size;
    virtual ~Track();

    long long GetType() const;
    long long GetNumber() const;
    unsigned long long GetUid() const;
    const char* GetNameAsUTF8() const;
    const char* GetCodecNameAsUTF8() const;
    const char* GetCodecId() const;
    const unsigned char* GetCodecPrivate(size_t&) const;
    bool GetLacing() const;

    const BlockEntry* GetEOS() const;

    struct Settings
    {
        long long start;
        long long size;
    };

    struct Info
    {
        long long type;
        long long number;
        unsigned long long uid;
        char* nameAsUTF8;
        char* codecId;
        unsigned char* codecPrivate;
        size_t codecPrivateSize;
        char* codecNameAsUTF8;
        bool lacing;
        Settings settings;

        Info();
        void Clear();
    };

    long GetFirst(const BlockEntry*&) const;
    long GetNext(const BlockEntry* pCurr, const BlockEntry*& pNext) const;
    virtual bool VetEntry(const BlockEntry*) const = 0;
    virtual long Seek(long long time_ns, const BlockEntry*&) const = 0;

protected:
    Track(
        Segment*,
        const Info&,
        long long element_start,
        long long element_size);
    const Info m_info;

    class EOSBlock : public BlockEntry
    {
    public:
        EOSBlock();

        bool EOS() const;
        const Cluster* GetCluster() const;
        size_t GetIndex() const;
        const Block* GetBlock() const;
        bool IsBFrame() const;
    };

    EOSBlock m_eos;

};


class VideoTrack : public Track
{
    VideoTrack(const VideoTrack&);
    VideoTrack& operator=(const VideoTrack&);

public:
    VideoTrack(
        Segment*,
        const Info&,
        long long element_start,
        long long element_size);
    long long GetWidth() const;
    long long GetHeight() const;
    double GetFrameRate() const;

    bool VetEntry(const BlockEntry*) const;
    long Seek(long long time_ns, const BlockEntry*&) const;

private:
    long long m_width;
    long long m_height;
    double m_rate;

};


class AudioTrack : public Track
{
    AudioTrack(const AudioTrack&);
    AudioTrack& operator=(const AudioTrack&);

public:
    AudioTrack(
        Segment*,
        const Info&,
        long long element_start,
        long long element_size);
    double GetSamplingRate() const;
    long long GetChannels() const;
    long long GetBitDepth() const;
    bool VetEntry(const BlockEntry*) const;
    long Seek(long long time_ns, const BlockEntry*&) const;

private:
    double m_rate;
    long long m_channels;
    long long m_bitDepth;
};


class Tracks
{
    Tracks(const Tracks&);
    Tracks& operator=(const Tracks&);

public:
    Segment* const m_pSegment;
    const long long m_start;
    const long long m_size;
    const long long m_element_start;
    const long long m_element_size;

    Tracks(
        Segment*,
        long long start,
        long long size,
        long long element_start,
        long long element_size);
    virtual ~Tracks();

    const Track* GetTrackByNumber(unsigned long tn) const;
    const Track* GetTrackByIndex(unsigned long idx) const;

private:
    Track** m_trackEntries;
    Track** m_trackEntriesEnd;

    void ParseTrackEntry(
        long long,
        long long,
        Track*&,
        long long element_start,
        long long element_size);

public:
    unsigned long GetTracksCount() const;
};


class SegmentInfo
{
    SegmentInfo(const SegmentInfo&);
    SegmentInfo& operator=(const SegmentInfo&);

public:
    Segment* const m_pSegment;
    const long long m_start;
    const long long m_size;
    const long long m_element_start;
    const long long m_element_size;

    SegmentInfo(
        Segment*,
        long long start,
        long long size,
        long long element_start,
        long long element_size);

    ~SegmentInfo();

    long long GetTimeCodeScale() const;
    long long GetDuration() const;  //scaled
    const char* GetMuxingAppAsUTF8() const;
    const char* GetWritingAppAsUTF8() const;
    const char* GetTitleAsUTF8() const;

private:
    long long m_timecodeScale;
    double m_duration;
    char* m_pMuxingAppAsUTF8;
    char* m_pWritingAppAsUTF8;
    char* m_pTitleAsUTF8;
};


class SeekHead
{
    SeekHead(const SeekHead&);
    SeekHead& operator=(const SeekHead&);

public:
    Segment* const m_pSegment;
    const long long m_start;
    const long long m_size;
    const long long m_element_start;
    const long long m_element_size;

    SeekHead(
        Segment*,
        long long start,
        long long size,
        long long element_start,
        long long element_size);

    ~SeekHead();

    struct Entry
    {
        long long id;
        long long pos;
    };

    int GetCount() const;
    const Entry* GetEntry(int idx) const;

private:
    Entry* m_entries;
    int m_count;

    static void ParseEntry(
        IMkvReader*,
        long long pos,
        long long size,
        Entry*&);

};

class Cues;
class CuePoint
{
    friend class Cues;

    CuePoint(size_t, long long);
    ~CuePoint();

    CuePoint(const CuePoint&);
    CuePoint& operator=(const CuePoint&);

public:
    long long m_element_start;
    long long m_element_size;

    void Load(IMkvReader*);

    long long GetTimeCode() const;      //absolute but unscaled
    long long GetTime(const Segment*) const;  //absolute and scaled (ns units)

    struct TrackPosition
    {
        long long m_track;
        long long m_pos;  //of cluster
        long long m_block;
        //codec_state  //defaults to 0
        //reference = clusters containing req'd referenced blocks
        //  reftime = timecode of the referenced block

        void Parse(IMkvReader*, long long, long long);
    };

    const TrackPosition* Find(const Track*) const;

private:
    const size_t m_index;
    long long m_timecode;
    TrackPosition* m_track_positions;
    size_t m_track_positions_count;

};


class Cues
{
    friend class Segment;

    Cues(
        Segment*,
        long long start,
        long long size,
        long long element_start,
        long long element_size);
    ~Cues();

    Cues(const Cues&);
    Cues& operator=(const Cues&);

public:
    Segment* const m_pSegment;
    const long long m_start;
    const long long m_size;
    const long long m_element_start;
    const long long m_element_size;

    bool Find(  //lower bound of time_ns
        long long time_ns,
        const Track*,
        const CuePoint*&,
        const CuePoint::TrackPosition*&) const;

#if 0
    bool FindNext(  //upper_bound of time_ns
        long long time_ns,
        const Track*,
        const CuePoint*&,
        const CuePoint::TrackPosition*&) const;
#endif

    const CuePoint* GetFirst() const;
    const CuePoint* GetLast() const;
    const CuePoint* GetNext(const CuePoint*) const;

    const BlockEntry* GetBlock(
                        const CuePoint*,
                        const CuePoint::TrackPosition*) const;

    bool LoadCuePoint() const;
    long GetCount() const;  //loaded only
    //long GetTotal() const;  //loaded + preloaded
    bool DoneParsing() const;

private:
    void Init() const;
    void PreloadCuePoint(size_t&, long long) const;

    mutable CuePoint** m_cue_points;
    mutable size_t m_count;
    mutable size_t m_preload_count;
    mutable long long m_pos;

};


class Cluster
{
    friend class Segment;

    Cluster(const Cluster&);
    Cluster& operator=(const Cluster&);

public:
    Segment* const m_pSegment;

public:
    static Cluster* Create(
        Segment*,
        long index,     //index in segment
        long long off,  //offset relative to segment
        long long element_size);

    Cluster();  //EndOfStream
    ~Cluster();

    bool EOS() const;

    long long GetTimeCode() const;   //absolute, but not scaled
    long long GetTime() const;       //absolute, and scaled (nanosecond units)
    long long GetFirstTime() const;  //time (ns) of first (earliest) block
    long long GetLastTime() const;   //time (ns) of last (latest) block

    const BlockEntry* GetFirst() const;
    const BlockEntry* GetLast() const;
    const BlockEntry* GetNext(const BlockEntry*) const;
    const BlockEntry* GetEntry(const Track*, long long ns = -1) const;
    const BlockEntry* GetEntry(
        const CuePoint&,
        const CuePoint::TrackPosition&) const;
    const BlockEntry* GetMaxKey(const VideoTrack*) const;

//    static bool HasBlockEntries(const Segment*, long long);

    static long HasBlockEntries(
            const Segment*,
            long long idoff,
            long long& pos,
            long& size);

    long GetEntryCount() const;

    void Load() const;
    long Load(long long& pos, long& size) const;

    void LoadBlockEntries() const;

    long Parse(long long& pos, long& size) const;
    long GetEntry(long index, const mkvparser::BlockEntry*&) const;

protected:
    Cluster(
        Segment*,
        long index,
        //long long off,
        long long element_start,
        long long element_size);

public:
    const long long m_element_start;
    long long GetPosition() const;  //offset relative to segment

    long GetIndex() const;
    long long GetElementSize() const;
    //long long GetPayloadSize() const;

    long long Unparsed() const;

private:
    long m_index;
    mutable long long m_pos;
    mutable long long m_size;
    mutable long long m_element_size;
    mutable long long m_timecode;
    mutable BlockEntry** m_entries;
    mutable long m_entries_size;
    mutable long m_entries_count;

    void ParseBlock(long long id, long long pos, long long size) const;
    void ParseBlockGroup(long long, long long, BlockEntry**&) const;
    void ParseSimpleBlock(long long, long long, BlockEntry**&) const;

};


class Segment
{
    friend class Cues;
    friend class VideoTrack;
    friend class AudioTrack;

    Segment(const Segment&);
    Segment& operator=(const Segment&);

private:
    Segment(IMkvReader*, long long pos, long long size);

public:
    IMkvReader* const m_pReader;
    const long long m_start;  //posn of segment payload
    const long long m_size;   //size of segment payload
    Cluster m_eos;  //TODO: make private?

    static long long CreateInstance(IMkvReader*, long long, Segment*&);
    ~Segment();

    long Load();  //loads headers and all clusters

    //for incremental loading
    long long Unparsed() const;
    long long ParseHeaders();  //stops when first cluster is found
    //long FindNextCluster(long long& pos, long& size) const;
    long LoadCluster(long long& pos, long& size);  //load one cluster
    long LoadCluster();

    long ParseNext(
            const Cluster* pCurr,
            const Cluster*& pNext,
            long long& pos,
            long& size);

#if 0
    //This pair parses one cluster, but only changes the state of the
    //segment object when the cluster is actually added to the index.
    long ParseCluster(long long& cluster_pos, long long& new_pos) const;
    bool AddCluster(long long cluster_pos, long long new_pos);
#endif

    const SeekHead* GetSeekHead() const;
    const Tracks* GetTracks() const;
    const SegmentInfo* GetInfo() const;
    const Cues* GetCues() const;

    long long GetDuration() const;

    unsigned long GetCount() const;
    const Cluster* GetFirst() const;
    const Cluster* GetLast() const;
    const Cluster* GetNext(const Cluster*);

    const Cluster* FindCluster(long long time_nanoseconds) const;
    //const BlockEntry* Seek(long long time_nanoseconds, const Track*) const;

    const Cluster* FindOrPreloadCluster(long long pos);

    long ParseCues(
        long long cues_off,  //offset relative to start of segment
        long long& parse_pos,
        long& parse_len);

private:

    long long m_pos;  //absolute file posn; what has been consumed so far

    SeekHead* m_pSeekHead;
    SegmentInfo* m_pInfo;
    Tracks* m_pTracks;
    Cues* m_pCues;
    Cluster** m_clusters;
    long m_clusterCount;         //number of entries for which m_index >= 0
    long m_clusterPreloadCount;  //number of entries for which m_index < 0
    long m_clusterSize;          //array size

    void AppendCluster(Cluster*);
    void PreloadCluster(Cluster*, ptrdiff_t);

    //void ParseSeekHead(long long pos, long long size);
    //void ParseSeekEntry(long long pos, long long size);
    //void ParseCues(long long);

    const BlockEntry* GetBlock(
        const CuePoint&,
        const CuePoint::TrackPosition&);

};

}  //end namespace mkvparser

inline long mkvparser::Segment::LoadCluster()
{
    long long pos;
    long size;

    return LoadCluster(pos, size);
}

#endif  //MKVPARSER_HPP
