package decode

import (
	"bufio"
	"encoding/binary"
	"fmt"
	"os"
	"strings"
)

var patterns = [256]int{
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	0,  /* 00..00.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	1,  /* 00..01.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	2,  /* 00..10.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	3,  /* 00..11.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	4,  /* 01..00.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	5,  /* 01..01.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	6,  /* 01..10.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	7,  /* 01..11.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	8,  /* 10..00.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	9,  /* 10..01.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	10, /* 10..10.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	11, /* 10..11.. */
	12, /* 110.000. */
	12, /* 110.000. */
	13, /* 110.001. */
	13, /* 110.001. */
	14, /* 110.010. */
	14, /* 110.010. */
	15, /* 110.011. */
	15, /* 110.011. */
	16, /* 110.100. */
	16, /* 110.100. */
	17, /* 110.101. */
	17, /* 110.101. */
	18, /* 110.110. */
	18, /* 110.110. */
	19, /* 110.111. */
	19, /* 110.111. */
	12, /* 110.000. */
	12, /* 110.000. */
	13, /* 110.001. */
	13, /* 110.001. */
	14, /* 110.010. */
	14, /* 110.010. */
	15, /* 110.011. */
	15, /* 110.011. */
	16, /* 110.100. */
	16, /* 110.100. */
	17, /* 110.101. */
	17, /* 110.101. */
	18, /* 110.110. */
	18, /* 110.110. */
	19, /* 110.111. */
	19, /* 110.111. */
	20, /* 111.000. */
	20, /* 111.000. */
	21, /* 111.001. */
	21, /* 111.001. */
	22, /* 111.010. */
	22, /* 111.010. */
	23, /* 111.011. */
	23, /* 111.011. */
	24, /* 111.100. */
	24, /* 111.100. */
	25, /* 11101010 */
	26, /* 11101011 */
	27, /* 11101100 */
	28, /* 11101101 */
	29, /* 11101110 */
	30, /* 11101111 */
	20, /* 111.000. */
	20, /* 111.000. */
	21, /* 111.001. */
	21, /* 111.001. */
	22, /* 111.010. */
	22, /* 111.010. */
	23, /* 111.011. */
	23, /* 111.011. */
	24, /* 111.100. */
	24, /* 111.100. */
	31, /* 11111010 */
	32, /* 11111011 */
	33, /* 11111100 */
	34, /* 11111101 */
	35, /* 11111110 */
	36, /* 11111111 */
}

var symbolIndex = map[byte]int{
	'p': -1,
	'1': 0,
	'2': 1,
	'3': 2,
	'4': 3,
	'5': 4,
	'6': 5,
	'7': 6,
	'8': 7,
	'9': 8,
	'a': 9,
	'b': 10,
	'c': 11,
	'd': 12,
	'e': 13,
	'f': 14,
	'g': 15,
}

var decoratedPatternA = "ppp88888 88888888 pppggggg gggggggg 87777776 66666655 gffffffe eeeeeedd 55554444 44444333 ddddcccc cccccbbb 33322222 22111111 bbbaaaaa aa999999"
var decoratedPatternB = "pp888888 88888887 ppgggggg gggggggf 77777666 66666555 fffffeee eeeeeddd 55544444 44443333 dddccccc ccccbbbb 33222222 22111111 bbaaaaaa aa999999"
var decoratedPatternB3 = "ppp88888 88888887 pppggggg gggggggf 77777666 66666555 fffffeee eeeeeddd 55544444 44443333 dddccccc ccccbbbb 33222222 22111111 bbaaaaaa aa999999"
var decoratedPatternB4 = "pppp8888 88888887 ppppgggg gggggggf 77777766 66666555 ffffffee eeeeeddd 55554444 44433333 ddddcccc cccbbbbb 33222222 21111111 bbaaaaaa a9999999"
var decoratedPatternC = "ppp88888 88888877 pppggggg ggggggff 77776666 66665555 ffffeeee eeeedddd 55444444 44443333 ddcccccc ccccbbbb 33222222 22111111 bbaaaaaa aa999999"
var decoratedPatternD = "pp888888 88877777 ppgggggg gggfffff 77666666 66555555 ffeeeeee eedddddd 54444444 44333333 dccccccc ccbbbbbb 32222222 21111111 baaaaaaa a9999999"
var decoratedPatternE = "pppp8888 88888877 ppppgggg ggggggff 77776666 66665555 ffffeeee eeeedddd 55444444 44443333 ddcccccc ccccbbbb 33222222 22111111 bbaaaaaa aa999999"
var decoratedPatternF = "pppp8888 88887777 ppppgggg ggggffff 77766666 66655555 fffeeeee eeeddddd 55444444 44333333 ddcccccc ccbbbbbb 32222222 21111111 baaaaaaa a9999999"

var decoratedPattern12A = "pp888888 88888777 ppgggggg gggggfff 76666665 55544444 feeeeeed dddccccc 44333322 22221111 ccbbbbaa aaaa9999"
var decoratedPattern12B = "pp888888 87777766 ppgggggg gfffffee 66665555 54444444 eeeedddd dccccccc 33333222 22211111 bbbbbaaa aaa99999"
var decoratedPattern12C = "pppp8888 88777776 ppppgggg ggfffffe 66666555 55444444 eeeeeddd ddcccccc 33333222 22211111 bbbbbaaa aaa99999"
var decoratedPattern12D = "pppp8888 88887777 ppppgggg ggggffff 66666655 55444444 eeeeeedd ddcccccc 44333322 22221111 ccbbbbaa aaaa9999"
var decoratedPattern12E = "ppp88888 88887777 pppggggg ggggffff 66666655 55444444 eeeeeedd ddcccccc 44333322 22221111 ccbbbbaa aaaa9999"
var decoratedPattern12F = "ppp88888 88888887 pppggggg gggggggf 77766666 55554444 fffeeeee ddddcccc 44433332 22221111 cccbbbba aaaa9999"
var decoratedPattern12G = "ppp88888 88888777 pppggggg gggggfff 76666665 55544444 feeeeeed dddccccc 44333322 22221111 ccbbbbaa aaaa9999"

var patternA = stripSpaces(decoratedPatternA)
var patternB = stripSpaces(decoratedPatternB)
var patternB3 = stripSpaces(decoratedPatternB3)
var patternB4 = stripSpaces(decoratedPatternB4)
var patternC = stripSpaces(decoratedPatternC)
var patternD = stripSpaces(decoratedPatternD)
var patternE = stripSpaces(decoratedPatternE)
var patternF = stripSpaces(decoratedPatternF)

var pattern12A = stripSpaces(decoratedPattern12A)
var pattern12B = stripSpaces(decoratedPattern12B)
var pattern12C = stripSpaces(decoratedPattern12C)
var pattern12D = stripSpaces(decoratedPattern12D)
var pattern12E = stripSpaces(decoratedPattern12E)
var pattern12F = stripSpaces(decoratedPattern12F)
var pattern12G = stripSpaces(decoratedPattern12G)

//*****************************************************************************
func main() {
	/*
		dir, err := os.Getwd()
		if err != nil {
			log.Fatal(err)
		}
		fmt.Println("directory = %s\n", dir)
	*/
	banner()

	songDir := "C:/testout/Partition0/SONG0000.VR5"
	reaperDir := "C:/testout/reaper"

	ext := songDir[len(songDir)-3:]

	format := getFormatForExtension(ext)

	fmt.Printf("System:      %s\n", format)

	switch format {
	//case "VS-1680":
	//    convertVS1680ToReaper(songDir, reaperDir)
	case "VS-1880":
		convertVS1880ToReaper(songDir, reaperDir)
	//case "VS-2480":
	//    convertVS2480ToReaper(songDir, reaperDir)
	default:
		fmt.Printf("Unsupported format: %s\n", format)
	}

	fmt.Printf("DONE\n")
}

//*****************************************************************************
func banner() {
	fmt.Printf("\n")
	fmt.Printf("**********************************************************\n")
	fmt.Printf("*                                                        *\n")
	fmt.Printf("*    Roland VS to Reaper Converter (Version 1.0)         *\n")
	fmt.Printf("*                                                        *\n")
	fmt.Printf("*    Copyright 2017 Randy Gordon (randy@integrand.com)   *\n")
	fmt.Printf("*                                                        *\n")
	fmt.Printf("**********************************************************\n")
	fmt.Printf("\n")
}

//*****************************************************************************
func convertMTP(inFileName string, outFileName string, sampleRate int, bitDepth int) {

	// Open input file
	fin, err := os.Open(inFileName)
	check(err)
	defer fin.Close()
	finInfo, err := fin.Stat()
	check(err)

	// Open output file
	fout, err := os.Create(outFileName)
	check(err)
	defer fout.Close()

	fileSize := finInfo.Size()
	numBlocks := int(fileSize / 16)

	// Write the WAV header
	numSamples := numBlocks * 16
	writeWavHeader(fout, numSamples, sampleRate, bitDepth, 1)

	in := make([]uint8, 16)
	out := make([]int, 16)
	d0 := 0

	r := bufio.NewReader(fin)

	for i := 0; i < numBlocks; i++ {
		_, err := r.Read(in)
		check(err)
		DecodeMTP(d0, in, out)
		writeWavSamples24(fout, out)
		d0 = out[15]

		if i%50000 == 0 {
			fmt.Printf(".")
		}
	}
}

//*****************************************************************************
func writeWavHeader(fout *os.File, numSamples int, sampleRate int, bitDepth int, numChannels int) {

	w := bufio.NewWriter(fout)

	bytesPerSample := bitDepth / 8
	numBytes := numSamples * bytesPerSample * numChannels
	numChunkBytes := numBytes + 38

	chunkid := []byte{0x52, 0x49, 0x46, 0x46}

	w.Write(chunkid)

	chunksize := []byte{
		BYTE_0(numChunkBytes),
		BYTE_1(numChunkBytes),
		BYTE_2(numChunkBytes),
		BYTE_3(numChunkBytes),
	}

	w.Write(chunksize)

	format := []byte{0x57, 0x41, 0x56, 0x45}
	w.Write(format)

	subchunk1id := []byte{0x66, 0x6d, 0x74, 0x20}
	w.Write(subchunk1id)

	subchunk1size := []byte{0x12, 0x00, 0x00, 0x00}
	w.Write(subchunk1size)

	audioformat := []byte{0x01, 0x00}
	w.Write(audioformat)

	numchannels := []byte{
		BYTE_0(numChannels),
		BYTE_1(numChannels),
	}
	w.Write(numchannels)

	samplerate := []byte{
		BYTE_0(sampleRate),
		BYTE_1(sampleRate),
		BYTE_2(sampleRate),
		BYTE_3(sampleRate),
	}
	w.Write(samplerate)

	byterate := []byte{
		BYTE_0(sampleRate * bytesPerSample * numChannels),
		BYTE_1(sampleRate * bytesPerSample * numChannels),
		BYTE_2(sampleRate * bytesPerSample * numChannels),
		BYTE_3(sampleRate * bytesPerSample * numChannels),
	}
	w.Write(byterate)

	blockalign := []byte{
		BYTE_0(bytesPerSample * numChannels),
		BYTE_1(bytesPerSample * numChannels),
	}
	w.Write(blockalign)

	bitspersample := []byte{
		BYTE_0(bitDepth),
		BYTE_1(bitDepth),
	}
	w.Write(bitspersample)

	extraparamsize := []byte{0x00, 0x00}
	w.Write(extraparamsize)

	subchunk2id := []byte{0x64, 0x61, 0x74, 0x61}
	w.Write(subchunk2id)

	subchunk2size := []byte{
		BYTE_0(numBytes),
		BYTE_1(numBytes),
		BYTE_2(numBytes),
		BYTE_3(numBytes),
	}
	w.Write(subchunk2size)

	w.Flush()
}

//*****************************************************************************
func writeWavSamples24(fout *os.File, samples []int) {
	// Write it to the WAV (little-endian)
	w := bufio.NewWriter(fout)
	for i := 0; i < 16; i++ {
		sample := []byte{
			BYTE_0(samples[i]),
			BYTE_1(samples[i]),
			BYTE_2(samples[i]),
		}
		w.Write(sample)
	}
	w.Flush()
}

//*****************************************************************************
func DecodeMTP(d0 int, in []uint8, out []int) {

	// Decodes a 16-byte MTP RDAC block into 24-bit samples.

	patternIndex := (in[0] & 0xf0) | ((in[2] & 0xf0) >> 4)

	pattern := patterns[patternIndex]

	for i := 0; i < 16; i++ {
		out[i] = 0
	}
	//fmt.Printf("pattern = %d\n", pattern)
	switch pattern {

	// Pattern B

	case 0:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 1:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 7)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 2:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 3:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 9)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 4:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 10)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 5:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 11)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern D

	case 6:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 10)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 7:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 11)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 8:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 12)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 9:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 13)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 10:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 14)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 11:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 15)
		// 4 linear
		INTERPOLATE_4(d0, out)

		// Pattern A

	case 12:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 13:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 14:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 7)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 15:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 16:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 9)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 17:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 10)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern B3

	case 18:
		patternStr := patternB3
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 12)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern C

	case 19:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 20:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 9)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 21:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 10)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 22:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 11)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 23:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 12)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 24:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 13)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern F

	case 25:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 12)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 26:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 13)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 27:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 14)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 28:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 15)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 29:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 16)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 30:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 16)
		// 16 linear - but odd samples are doubled!
		DOUBLE_ODDS(out)

		// Pattern E

	case 31:
		patternStr := patternE
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 14)
		// 4 linear
		INTERPOLATE_4(d0, out)

		// Pattern B4

	case 32:
		patternStr := patternB4
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 33:
		patternStr := patternB4
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 34:
		patternStr := patternB4
		applyPattern(in, out, patternStr)
		// None: SHIFT_ROUND()
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 35:
		patternStr := patternB4
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 36:
		patternStr := patternB4
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 3)
		// 2 linear
		INTERPOLATE_2(d0, out)
	default:
	}

	PREVENT_OVERFLOW_24(out)
}

//*****************************************************************************
func DecodeMT1(d0 int, in []uint8, out []int) {

	// Decodes a 16-byte MT1 RDAC block into 16-bit samples.

	patternIndex := (in[0] & 0xf0) | ((in[2] & 0xf0) >> 4)

	pattern := patterns[patternIndex]

	for i := 0; i < 16; i++ {
		out[i] = 0
	}
	//fmt.Printf("pattern = %d\n", pattern)
	switch pattern {

	// Pattern B

	case 0:
		/* Unknown - never occurs? */
		//        fmt.Printf("DANGER! %d %v\n",pattern, in)
	case 1:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 2:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 3:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 1)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 4:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 5:
		patternStr := patternB
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 3)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern D

	case 6:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 7:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 3)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 8:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 9:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 10:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 11:
		patternStr := patternD
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 7)
		// 4 linear
		INTERPOLATE_4(d0, out)

		// Pattern A

	case 12:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 13:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 14:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 15:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 16:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 1)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 17:
		patternStr := patternA
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern B3

	case 18:
		patternStr := patternB3
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern C

	case 19:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 20:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 1)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 21:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 22:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 3)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 23:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 24:
		patternStr := patternC
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern F

	case 25:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 26:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 27:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 28:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 7)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 29:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 30:
		patternStr := patternF
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 16 linear - but odd samples are doubled!
		DOUBLE_ODDS(out)

		// Pattern E

	case 31:
		patternStr := patternE
		applyPattern(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 4 linear
		INTERPOLATE_4(d0, out)

		// Pattern B4

	case 32:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 33:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 34:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 35:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 36:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	default:
	}

	PREVENT_OVERFLOW_16(out)
}

//*****************************************************************************
func DecodeMT2(d0 int, in []uint8, out []int) {

	// Decodes a 16-byte MT2 RDAC block into 16-bit samples.

	patternIndex := (in[0] & 0xf0) | ((in[2] & 0xf0) >> 4)

	pattern := patterns[patternIndex]

	for i := 0; i < 16; i++ {
		out[i] = 0
	}

	switch pattern {

	// Pattern 12A

	case 0:
		patternStr := pattern12A
		applyPattern12(in, out, patternStr)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 1:
		patternStr := pattern12A
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 1)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 2:
		patternStr := pattern12A
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 3:
		patternStr := pattern12A
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 3)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 4:
		patternStr := pattern12A
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 5:
		patternStr := pattern12A
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern 12B

	case 6:
		patternStr := pattern12B
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 7:
		patternStr := pattern12B
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 8:
		patternStr := pattern12B
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 9:
		patternStr := pattern12B
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 7)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 10:
		patternStr := pattern12B
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 4 linear
		INTERPOLATE_4(d0, out)
	case 11:
		patternStr := pattern12B
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 9)
		// 4 linear
		INTERPOLATE_4(d0, out)

		// Pattern 12F

	case 12:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 13:
		patternStr := pattern12F
		applyPattern12(in, out, patternStr)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 14:
		patternStr := pattern12F
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 1)
		// 4 linear
		INTERPOLATE_2(d0, out)
	case 15:
		patternStr := pattern12F
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 4 linear
		INTERPOLATE_2(d0, out)
	case 16:
		patternStr := pattern12F
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 3)
		// 4 linear
		INTERPOLATE_2(d0, out)
	case 17:
		patternStr := pattern12F
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 4 linear
		INTERPOLATE_2(d0, out)

		// Pattern 12G

	case 18:
		patternStr := pattern12G
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern 12E

	case 19:
		patternStr := pattern12E
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 2)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 20:
		patternStr := pattern12E
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 3)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 21:
		patternStr := pattern12E
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 4)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 22:
		patternStr := pattern12E
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 5)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 23:
		patternStr := pattern12E
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 2 linear
		INTERPOLATE_2(d0, out)
	case 24:
		patternStr := pattern12E
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 7)
		// 2 linear
		INTERPOLATE_2(d0, out)

		// Pattern 12C

	case 25:
		patternStr := pattern12C
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 6)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 26:
		patternStr := pattern12C
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 7)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 27:
		patternStr := pattern12C
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 28:
		patternStr := pattern12C
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 9)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 29:
		patternStr := pattern12C
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 10)
		// 8 linear
		INTERPOLATE_8(d0, out)
	case 30:
		patternStr := pattern12C
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 10)
		// 16 linear - but odd samples are doubled!
		DOUBLE_ODDS(out)

		// Pattern 12D

	case 31:
		patternStr := pattern12D
		applyPattern12(in, out, patternStr)
		SHIFT_ROUND(out, 8)
		// 4 linear
		INTERPOLATE_4(d0, out)

		// Pattern ?

	case 32:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 33:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 34:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 35:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	case 36:
		/* Unknown - never occurs? */
		fmt.Printf("DANGER! %d %v\n", pattern, in)
	default:
	}

	PREVENT_OVERFLOW_16(out)
}

//*****************************************************************************
func DecodeM16(d0 int, in []uint8, out []int) {

	// Decodes a 32-byte M16 block into 16 16-bit samples.

	for i := 0; i < 16; i++ {
		out[i] = int(uint(in[i*2])<<8 | uint(in[i*2+1]))
		out[i] = SIGN_EXTEND(out[i], 15)
	}
}

//*****************************************************************************
func DecodeM24(d0 int, in []uint8, out []int) {

	// Decodes a 48-byte M24 block into 16 24-bit samples.

	for i := 0; i < 16; i++ {
		out[i] = int(uint(in[i*3])<<16 | uint(in[i*3])<<8 | uint(in[i*3+1]))
		out[i] = SIGN_EXTEND(out[i], 23)
	}
}

//*****************************************************************************
func applyPattern(in []uint8, out []int, pattern string) {

	outPos := make([]uint, 16)

	for inPosition := 15; inPosition >= 0; inPosition-- {
		bytePatternIndex := inPosition * 8
		bytePatternStr := pattern[bytePatternIndex : bytePatternIndex+8]

		for bitPosition := 0; bitPosition <= 7; bitPosition++ {
			symbol := bytePatternStr[7-bitPosition]

			//fmt.Printf("%s\n",string(symbol))
			outIndex := symbolIndex[symbol]
			if outIndex == -1 {
				continue
			}

			hasBit := ((in[inPosition] >> uint(bitPosition)) & 0x01) == 0x01

			if hasBit {
				out[outIndex] |= 0x01 << outPos[outIndex]
			}
			outPos[outIndex]++
		}
	}
	// Sign extend
	for i := 0; i < 16; i++ {
		//		fmt.Printf("   %032b - %d\n", toUint32(out[i]), outPos[i]-1)
		out[i] = SIGN_EXTEND(out[i], outPos[i]-1)
		//		fmt.Printf("x  %032b - %d\n", toUint32(out[i]), outPos[i]-1)
	}
}

//*****************************************************************************
func applyPattern12(in []uint8, out []int, pattern string) {

	outPos := make([]uint, 16)

	for inPosition := 11; inPosition >= 0; inPosition-- {
		bytePatternIndex := inPosition * 8
		bytePatternStr := pattern[bytePatternIndex : bytePatternIndex+8]

		for bitPosition := 0; bitPosition <= 7; bitPosition++ {
			symbol := bytePatternStr[7-bitPosition]

			//fmt.Printf("%s\n",string(symbol))
			outIndex := symbolIndex[symbol]
			if outIndex == -1 {
				continue
			}

			hasBit := ((in[inPosition] >> uint(bitPosition)) & 0x01) == 0x01

			if hasBit {
				out[outIndex] |= 0x01 << outPos[outIndex]
			}
			outPos[outIndex]++
		}
	}
	// Sign extend
	for i := 0; i < 16; i++ {
		//		fmt.Printf("   %032b - %d\n", toUint32(out[i]), outPos[i]-1)
		out[i] = SIGN_EXTEND(out[i], outPos[i]-1)
		//		fmt.Printf("x  %032b - %d\n", toUint32(out[i]), outPos[i]-1)
	}
}

//*****************************************************************************
func check(e error) {
	if e != nil {
		panic(e)
	}
}

//*****************************************************************************
func BYTE_0(xx int) byte { return byte(xx & 0xff) }
func BYTE_1(xx int) byte { return byte((xx >> 8) & 0xff) }
func BYTE_2(xx int) byte { return byte((xx >> 16) & 0xff) }
func BYTE_3(xx int) byte { return byte((xx >> 24) & 0xff) }

func SIGN_EXTEND(xx int, signPos uint) int { return -(xx & (0x01 << (signPos))) | xx }

func SHIFT_ROUND(out []int, pos uint) {
	for i := 0; i < 16; i++ {
		out[i] = out[i]<<pos | 0x01<<(pos-1)
	}
}

func INTERPOLATE(aa int, bb int) int {
	if (aa + bb) < 0 {
		return (aa + bb - 1) / 2
	} else {
		return (aa + bb) / 2
	}
}

func INTERPOLATE_2(d0 int, out []int) {

	out[3] += INTERPOLATE(d0, out[7])
	out[1] += INTERPOLATE(d0, out[3])
	out[5] += INTERPOLATE(out[3], out[7])
	out[11] += INTERPOLATE(out[7], out[15])
	out[9] += INTERPOLATE(out[7], out[11])
	out[13] += INTERPOLATE(out[11], out[15])
	out[0] += INTERPOLATE(d0, out[1])
	out[2] += INTERPOLATE(out[1], out[3])
	out[4] += INTERPOLATE(out[3], out[5])
	out[6] += INTERPOLATE(out[5], out[7])
	out[8] += INTERPOLATE(out[7], out[9])
	out[10] += INTERPOLATE(out[9], out[11])
	out[12] += INTERPOLATE(out[11], out[13])
	out[14] += INTERPOLATE(out[13], out[15])
}

func INTERPOLATE_4(d0 int, out []int) {

	out[1] += INTERPOLATE(d0, out[3])
	out[5] += INTERPOLATE(out[3], out[7])
	out[9] += INTERPOLATE(out[7], out[11])
	out[13] += INTERPOLATE(out[11], out[15])
	out[0] += INTERPOLATE(d0, out[1])
	out[2] += INTERPOLATE(out[1], out[3])
	out[4] += INTERPOLATE(out[3], out[5])
	out[6] += INTERPOLATE(out[5], out[7])
	out[8] += INTERPOLATE(out[7], out[9])
	out[10] += INTERPOLATE(out[9], out[11])
	out[12] += INTERPOLATE(out[11], out[13])
	out[14] += INTERPOLATE(out[13], out[15])
}

func INTERPOLATE_8(d0 int, out []int) {
	out[0] += INTERPOLATE(d0, out[1])
	out[2] += INTERPOLATE(out[1], out[3])
	out[4] += INTERPOLATE(out[3], out[5])
	out[6] += INTERPOLATE(out[5], out[7])
	out[8] += INTERPOLATE(out[7], out[9])
	out[10] += INTERPOLATE(out[9], out[11])
	out[12] += INTERPOLATE(out[11], out[13])
	out[14] += INTERPOLATE(out[13], out[15])
}

func LIMIT_16(xx int) int {
	if xx < -32768 {
		return -32768
	} else if xx > 32767 {
		return 32767
	} else {
		return xx
	}
}

func LIMIT_24(xx int) int {
	if xx < -8388608 {
		return -8388608
	} else if xx > 8388607 {
		return 8388607
	} else {
		return xx
	}
}

func toUint32(xx int) uint32 {
	return uint32(xx & 0xffffffff)
	//    return uint32(uint32(xx) & 0xffffffff)
}

func PREVENT_OVERFLOW_16(out []int) {
	for i := 0; i < 16; i++ {
		out[i] = LIMIT_16(out[i])
	}
}

func PREVENT_OVERFLOW_24(out []int) {
	for i := 0; i < 16; i++ {
		out[i] = LIMIT_24(out[i])
	}
}

func DOUBLE_ODDS(out []int) {
	for i := 0; i < 16; i += 2 {
		out[i] <<= 1
	}
}

//*****************************************************************************
func stripSpaces(s string) string {
	s = strings.Replace(s, " ", "", -1)
	return s
}

//*****************************************************************************
func showPattern(s string) {
	fmt.Printf("%s\n", s)
}

//*****************************************************************************
func showBits8(out []uint8) {
	for i := 0; i < 16; i++ {
		fmt.Printf("%08b", out[i])
	}
	fmt.Printf("\n")
}

//*****************************************************************************
//func showBits32(out []uint32) {
func showBits32(out []int) {
	for i := 0; i < 16; i++ {
		fmt.Printf("%b ", toUint32(out[i]))
	}
	fmt.Printf("\n")
}

//*****************************************************************************
func showInts(out []int) {
	for i := 0; i < 16; i++ {
		fmt.Printf("%d ", out[i])
	}
	fmt.Printf("\n")
}

//*****************************************************************************
func showBits(out uint32) {
	fmt.Printf("%08b", out)
	fmt.Printf("\n")
}

//*****************************************************************************
func getSongInfo(vsFormat string, songFilePath string) (string, int, int) {
	f, err := os.Open(songFilePath)
	check(err)
	defer f.Close()

	var songName [12]byte
	var rate [1]byte
	var format [1]byte

	if getFormat(format[0]) == "VS-2480" {
		f.Seek(10, 0)
		f.Read(songName[:])

		f.Seek(22, 0)
		f.Read(rate[:])
		f.Read(format[:])
	} else {
		f.Seek(6, 0)
		f.Read(songName[:])

		f.Seek(18, 0)
		f.Read(rate[:])
		f.Read(format[:])
	}

	return string(songName[:]), int(rate[0]), int(format[0])
}

//*****************************************************************************
func getSampleRate(rate int) float64 {
	switch rate {
	case 0:
		return 48000
	case 1:
		return 44100
	case 2:
		return 32000
	case 3:
		return 96000
	case 4:
		return 88200
	default:
		return 64000
	}
}

//*****************************************************************************
func getModeName(mode int) string {
	switch mode {
	case 0:
		return "MT1"
	case 1:
		return "MT2"
	case 2:
		return "LIV"
	case 3:
		return "M16"
	case 4:
		return "CDR"
	case 5:
		return "MTP"
	case 6:
		return "LV2"
	case 7:
		return "VSR"
	case 8:
		return "M24"
	default:
		return "UNKNOWN"
	}
}

//*****************************************************************************
func getFormat(format byte) string {
	switch format {
	case 1:
		return "VS-1680"
	case 2:
		return "VS-1880"
	case 3:
		return "VS-2480"
	case 4:
		return "VS-880"
	case 5:
		return "VS-880EX"
	case 6:
		return "VS-890"
	case 7:
		return "VS-840"
	case 8:
		return "VS-840GX"
	default:
		return "UNKNOWN"
	}
}

//*****************************************************************************
func getFormatForExtension(ext string) string {
	switch ext {
	case "VR6":
		return "VS-1680"
	case "VR5":
		return "VS-1880"
	case "VR1":
		return "VS-2480"
	case "VR9":
		return "VS-880EX"
	case "VR7":
		return "VS-890"
	case "VR4":
		return "VS-840GX"
	case "VR8":
		// It could be a VS-880 or a VS-840!
		// Heuristic: If SYSTEM.VS1 exists, its a VS-840
		// TBD
		return "VS-840"
	default:
		return "UNKNOWN"
	}
}

//*****************************************************************************
func convertVS1880ToReaper(songDirPath string, wavDirPath string) {

	ext := songDirPath[len(songDirPath)-3:]

	bitDepth := 24

	// Create the target directory for the Reaper conversion
	os.MkdirAll(wavDirPath, os.ModePerm)

	// Get song and event file paths.

	songFilePath := fmt.Sprintf("%s/SONG.%s", songDirPath, ext)
	eventFilePath := fmt.Sprintf("%s/EVENTLST.%s", songDirPath, ext)

	// Determine sample rate from song file.

	songName, rate, mode := getSongInfo("VS-1880", songFilePath)

	if "MTP" != getModeName(mode) {
		fmt.Printf("%s encodings are currently not supported.\n", getModeName(mode))
		return
	}

	// Open the Reaper project file to create
	//    reaperProject := fmt.Sprintf("%s/%s", wavDirPath, "vs.rpp")
	reaperProject := fmt.Sprintf("%s/%s.rpp", wavDirPath, songName)
	fout, err := os.Create(reaperProject)
	check(err)
	defer fout.Close()
	reaperWriter := bufio.NewWriter(fout)
	defer reaperWriter.Flush()

	vsSampleRate := getSampleRate(rate)

	// Show some info
	fmt.Printf("Song:        %s\n", songName)
	fmt.Printf("Mode:        %s\n", getModeName(mode))
	fmt.Printf("Sample Rate: %d\n", int(vsSampleRate))
	fmt.Printf("Bit Depth:   %d\n", bitDepth)

	// Open the event file.

	f, err := os.Open(eventFilePath)
	check(err)
	defer f.Close()

	// Skip past phrases

	var numPhrasesBytes [2]byte
	f.Seek(16, 0)
	f.Read(numPhrasesBytes[:])

	numPhrases := int(binary.BigEndian.Uint16(numPhrasesBytes[:]))
	f.Seek(int64(18+numPhrases*64), 0)

	reaperWriter.WriteString("<REAPER_PROJECT 0.1\n")

	for trackIndex := 1; trackIndex <= 18; trackIndex++ {
		for vtrackIndex := 1; vtrackIndex <= 16; vtrackIndex++ {
			var trackNameBytes [16]byte
			f.Read(trackNameBytes[:])
			//trackName := string(trackNameBytes[:])

			var numEventsBytes [2]byte
			f.Read(numEventsBytes[:])
			numEvents := int(binary.BigEndian.Uint16(numEventsBytes[:]))

			if numEvents == 0 {
				continue
			}

			trackName := fmt.Sprintf("Track%d-%d", trackIndex, vtrackIndex)

			reaperWriter.WriteString("<TRACK\n")
			reaperWriter.WriteString(trackName + "\n")

			for i := 0; i < numEvents; i++ {

				var buff [64]byte

				f.Read(buff[:])

				eventStart := int(binary.BigEndian.Uint32(buff[0:4]))
				eventEnd := int(binary.BigEndian.Uint32(buff[4:8]))
				eventOffset := int(binary.BigEndian.Uint32(buff[8:12]))

				sampleStart := float64(eventStart * 16)
				sampleEnd := float64(eventEnd * 16)
				sampleOffset := float64(eventOffset * 16)

				startSecs := sampleStart / vsSampleRate
				endSecs := sampleEnd / vsSampleRate
				offsetSecs := sampleOffset / vsSampleRate

				eventLengthSecs := endSecs - startSecs

				takeFileId := int(binary.BigEndian.Uint16(buff[20:22]))
				takeFileName := fmt.Sprintf("TAKE%04X.%s", takeFileId, ext)
				takeFilePath := fmt.Sprintf("%s/%s", songDirPath, takeFileName)

				//eventName := string(buff[48:64])

				eventName := fmt.Sprintf("%s(%d)", trackName, i)

				wavFilePath := fmt.Sprintf("%s/%s.WAV", wavDirPath, eventName)

				reaperWriter.WriteString("<ITEM\n")
				reaperWriter.WriteString(fmt.Sprintf("POSITION %.8f\n", startSecs))
				reaperWriter.WriteString(fmt.Sprintf("LENGTH %.8f\n", eventLengthSecs))
				reaperWriter.WriteString(fmt.Sprintf("SOFFS %.8f\n", offsetSecs))
				reaperWriter.WriteString(fmt.Sprintf("FADEIN 1 0.01 %f\n", 0.0))
				reaperWriter.WriteString(fmt.Sprintf("FADEOUT 1 0.01 %f\n", 0.0))
				reaperWriter.WriteString(fmt.Sprintf("LOOP 0\n"))
				reaperWriter.WriteString(fmt.Sprintf("NAME \"%s\"\n", eventName))
				reaperWriter.WriteString(fmt.Sprintf("SRCFN \"%s\"\n", wavFilePath))

				fmt.Printf("Converting %s to %s", takeFilePath, wavFilePath)
				convertMTP(takeFilePath, wavFilePath, int(vsSampleRate), 24)
				fmt.Printf("\n")
			}
			reaperWriter.WriteString(">\n")
		}
	}
	reaperWriter.WriteString(">\n")
	fmt.Printf("Created Reaper project file: %s\n", reaperProject)
}
