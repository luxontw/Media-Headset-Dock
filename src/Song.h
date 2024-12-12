#ifndef SONG_H
class Song
{
public:
    void set_title(String title) { this->title_ = title; };
    void set_album(String album) { this->album_ = album; };
    void set_artist(String artist) { this->artist_ = artist; };
    void set_song_id(String song_id) { this->song_id_ = song_id; };
    void set_album_id(String song_id) { this->song_id_ = song_id; };
    void set_change(bool is_change) { this->is_change_ = is_change; };
    void operator=(Song song) { set_title(song.title_), set_album(song.album_), set_artist(song.artist_), set_song_id(song.song_id_), set_album_id(song.album_id_), set_change(song.is_change_); };
    const String title() const { return title_; };
    const String album() const { return album_; };
    const String artist() const { return artist_; };
    const String song_id() const { return song_id_; };
    const String album_id() const { return album_id_; };
    const bool IsChange() const { return is_change_; };

private:
    String title_ = "";
    String album_ = "";
    String artist_ = "";
    String song_id_ = "";
    String album_id_ = "";
    bool is_change_ = false;
};
#define SONG_H
#endif