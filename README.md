# TVB Downloader

TVB Downloader is a simple GTK3-based GUI application written in C that downloads
YouTube videos. The program uses `youtube-dl` under the hood, so make sure it is
installed and available in your `PATH`.

## Build

You will need the GTK3 development libraries and `pkg-config` installed.
Below are example package commands for common distributions:

### Debian / Ubuntu

```
sudo apt-get install build-essential libgtk-3-dev pkg-config youtube-dl
```

### Arch Linux

```
sudo pacman -S base-devel gtk3 youtube-dl
```

### Fedora

```
sudo dnf install gcc gtk3-devel pkgconf-pkg-config youtube-dl
```

### openSUSE

```
sudo zypper install gcc gtk3-devel pkg-config youtube-dl
```

### Gentoo

```
sudo emerge --ask x11-libs/gtk+ net-misc/youtube-dl
```

### Alpine

```
sudo apk add build-base gtk+3.0 youtube-dl
```

The application has been tested on these systems and should also work on other
Linux distributions that provide GTK3 and `youtube-dl`.

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

## Fast Download Options

You can override the downloader command via the `TVB_DOWNLOADER` environment
variable. This allows using alternative tools for improved performance. Five
robust options are:

- `youtube-dl` (default)
- `yt-dlp`
- `aria2c`
- `axel`
- `wget`

Example using `yt-dlp` with `aria2c` for multi-connection downloads:

```bash
TVB_DOWNLOADER="yt-dlp --external-downloader aria2c" ./tvb-downloader
```
