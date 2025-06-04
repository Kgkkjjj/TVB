# TVB Downloader

TVB Downloader is a simple GTK3-based GUI application written in C that downloads
YouTube videos. The program uses `youtube-dl` under the hood, so make sure it is
installed and available in your `PATH`.

## Build

You will need the GTK3 development libraries and `pkg-config` installed.

```
sudo apt-get install build-essential libgtk-3-dev pkg-config youtube-dl
```

To build the application run:

```
make
```

This will produce the `tvb-downloader` executable.

## Usage

Run the application and paste the YouTube URL into the text field, then press the
**Download** button. The output from `youtube-dl` will be displayed in the window.

The video will be downloaded in the current working directory using the default
`youtube-dl` settings.
