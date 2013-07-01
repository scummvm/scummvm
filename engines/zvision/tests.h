
namespace ZVision {

void tests() {
#if 0
	// Video test
	Video::VideoDecoder *videoDecoder = new ZorkAVIDecoder();
	if (videoDecoder && videoDecoder->loadFile("zassets/temple/t000a11c.avi")) {
		Common::List<Graphics::PixelFormat> formats;
		formats.push_back(videoDecoder->getPixelFormat());
		//initGraphics(640, 480, true, formats);

		playVideo(videoDecoder);
	}
#endif

	Common::File f;

#if 1
	// Image test

	//initGraphics(640, 480, true, &format);

	if (f.open(" zassets/global/GJDEB11C.TGA")) {
		Graphics::TGADecoder tga;
		if (!tga.loadStream(f))
			error("Error while reading TGA image");
		f.close();

		const Graphics::Surface *tgaSurface = tga.getSurface();

		Graphics::Surface *screen = g_system->lockScreen();
		for (uint16 y = 0; y < tgaSurface->h; y++)
			memcpy(screen->getBasePtr(0, y), tgaSurface->getBasePtr(0, y), tgaSurface->pitch);
		g_system->unlockScreen();

		tga.destroy();
	}


#endif

#if 0
	// Sound test
	if (f.open("zassets/castle/c000h9tc.raw")) {
		Audio::SeekableAudioStream *audioStream = makeRawZorkStream(&f, 22050, DisposeAfterUse::YES);
		Audio::SoundHandle handle;
		g_system->getMixer()->playStream(Audio::Mixer::kSFXSoundType, &handle, audioStream);
	}

#endif
}

} // End of namespace ZVision
