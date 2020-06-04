package vsio

import (
	"bufio"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"os"
	"strconv"
	"strings"
	"vs/decode"
	//"context"
)

const SECTOR_SIZE = 512
const SECTORS_PER_CLUSTER = 64
const SECTORS_PER_FAT = 256
const DIRECTORY = 16

type bootRecordType struct {
	formattingOS       string
	bytesPerSector     int
	sectorsPerCluster  int
	reservedSectors    int
	fatCopies          int
	maxRootEntries     int
	sectorsPerFAT      int
	sectorsInPart      int
	rootStartSector    int
	clusterStartSector int
	fatStartSector     int
}

type fileInfoType struct {
	name        string
	ext         string
	attr        int
	cluster     int
	size        int
	data        []byte
	dirFiles    []fileInfoType
	clusters    []int
	isDirectory bool
	bootRecord  bootRecordType
	fat         []byte
	partition   int
	songIndex   map[string]string
}

type contextType struct {
	file       *os.File
	bootRecord bootRecordType
	fat        []byte
	partition  int
}

var partitionSongIndexMap = make(map[int](map[string]string))

var vsDrive *os.File

var vsMachine string
var vsPartitionCount int
var vsPartitionSize int

//*****************************************************************************
func List() {
	allFiles := getList()
	showList(allFiles)
}

//*****************************************************************************
func ConvertFirst(songName string, targetDir string) {

	allFiles := getList()

	file, err := findSongFileByFriendlyName(allFiles, songName)
	check(err)

	switch vsMachine {
	case "VS-1680":
		convertVS1880ToReaper(file, targetDir)
	case "VS-1880":
		convertVS1880ToReaper(file, targetDir)
	default:
		fmt.Printf("Unsupported VS format: %s\n", vsMachine)
	}
}

//*****************************************************************************
func Convert(songName string, partition int, targetDir string) {

	allFiles := getList()

	file, err := findSongFileOnPartition(allFiles, songName, partition)
	check(err)

	switch vsMachine {
	case "VS-1680":
		convertVS1880ToReaper(file, targetDir)
	case "VS-1880":
		convertVS1880ToReaper(file, targetDir)
	default:
		fmt.Printf("Unsupported VS format: %s\n", vsMachine)
	}
}

//*****************************************************************************
func getList() []fileInfoType {

	// Find the attached VS drive, if any
	findRolandDrive()

	// Get the start sectors for each partition
	partitionSectors := getPartitionSectors(vsDrive)

	allFiles := getDirectoryEntriesForAllPartitions(partitionSectors)

	getSongIndexes(allFiles)

	return allFiles
}

//*****************************************************************************
func getDirectoryEntriesForAllPartitions(partitionSectors []int) []fileInfoType {

	allFiles := make([]fileInfoType, 0, 1000)

	vsPartitionSize = (partitionSectors[1] - partitionSectors[0]) * 512
	vsPartitionCount = 0

	vsPartitionGBs := int(vsPartitionSize / 1000000000)

	for partition := 0; partition < len(partitionSectors); partition++ {
		// Read the boot record for the partition
		bootRecord, err := getBootRecord(vsDrive, partitionSectors[partition])

		if err == nil {
			vsPartitionCount++
		} else {
			break
		}
		//showBootRecord(bootRecord);

		context := newContext(vsDrive, bootRecord)

		fat := getFAT(context)
		//showFAT(fat);

		context.fat = fat
		context.partition = partition

		// Get the root directory, and recurse to all (only one level!) subdirectories.
		files := getDirectoryEntries(context, bootRecord.rootStartSector, false)
		//showFileStructure(files);

		//file, err := findSongListOnPartition(files, partition)

		allFiles = append(allFiles, files...)
	}

	fileInfo := allFiles[0]
	vsMachine = getFormatForExtension(fileInfo.ext)

	fmt.Printf("VS Format:       %s\n", vsMachine)
	fmt.Printf("Partition Size:  %dGB\n", vsPartitionGBs)
	fmt.Printf("Partition Count: %d\n\n", vsPartitionCount)

	return allFiles
}

//*****************************************************************************
func newContext(file *os.File, bootRecord bootRecordType) contextType {
	var context contextType
	context.file = file
	context.bootRecord = bootRecord
	return context
}

//*****************************************************************************
func findRolandDrive() {
	// Find the Roland VS drive - look at up to 10 drives
	for i := 0; i < 10; i++ {
		drive := "\\\\.\\PhysicalDrive" + strconv.Itoa(i)
		file, err := os.Open(drive)
		if err != nil {
			continue
		}
		if isRolandDrive(file) {
			vsDrive = file
			fmt.Printf("\nFound Roland VS drive at: %s\n\n", drive)
			return
		}
	}

	abort("\nUnable to locate an attached Roland VS drive")

	//return nil
}

//*****************************************************************************
func isRolandDrive(file *os.File) bool {
	// Check for the signature
	buffer := readSector(file, 63)
	return "Roland" == string(buffer[3:9])
}

//*****************************************************************************
func getPartitionSectors(file *os.File) []int {

	partitionSectors := make([]int, 12)
	partitionOffset := 446

	buffer := readSector(file, 0)

	for i := 0; i < 4; i++ {
		firstSector := getInt32(buffer, partitionOffset+8)
		partitionSectors[i] = firstSector
		partitionOffset += 16
	}

	// Extrapolate extra four partitions
	delta := partitionSectors[1] - partitionSectors[0]

	nextPartitionSector := partitionSectors[3] + delta
	for i := 0; i < 8; i++ {
		partitionSectors[i+4] = nextPartitionSector
		nextPartitionSector += delta
	}

	return partitionSectors
}

//*****************************************************************************
func getBootRecord(file *os.File, sector int) (bootRecordType, error) {

	buffer := readSector(file, sector)

	var bootRecord bootRecordType

	bootRecord.formattingOS = string(buffer[3:9])

	if "Roland" != bootRecord.formattingOS {
		return bootRecord, errors.New("Invalid boot record")
	}

	bootRecord.bytesPerSector = getUint16(buffer, 11)
	bootRecord.sectorsPerCluster = getUint8(buffer, 13)
	bootRecord.reservedSectors = getUint16(buffer, 14)
	bootRecord.fatCopies = getUint8(buffer, 16)
	bootRecord.maxRootEntries = getUint16(buffer, 17)
	bootRecord.sectorsPerFAT = getUint16(buffer, 22)
	bootRecord.sectorsInPart = getUint32(buffer, 28)
	bootRecord.rootStartSector = sector + bootRecord.reservedSectors + int(bootRecord.fatCopies)*bootRecord.sectorsPerFAT
	bootRecord.clusterStartSector = bootRecord.rootStartSector + (bootRecord.maxRootEntries*32)/bootRecord.bytesPerSector
	bootRecord.fatStartSector = sector + bootRecord.reservedSectors

	return bootRecord, nil
}

//*****************************************************************************
func getFAT(context contextType) []byte {
	buffer := make([]byte, SECTOR_SIZE*SECTORS_PER_FAT)

	sector := context.bootRecord.fatStartSector

	pos := int64(sector * SECTOR_SIZE)
	context.file.Seek(pos, 0)

	_, err := io.ReadFull(context.file, buffer)
	if err != nil {
		check(err)
	}
	return buffer
}

//*****************************************************************************
func getDirectoryEntries(context contextType, sector int, isChildDirectory bool) []fileInfoType {
	done := false
	files := make([]fileInfoType, 0, 5000)

	isClusterFucked := false

	// First byte of first sector should be ".", if not the cluster is scrambled
	if isChildDirectory {
		buffer := readSector(vsDrive, sector)
		if buffer[0] == ' ' {
			isClusterFucked = true
		}
	}

	for {
		buffer := readSector(vsDrive, sector)

		if isClusterFucked {
			unClusterFuck(buffer)
		}

		// Up to 16 32-byte entries per sector.
		for i := 0; i < 16; i++ {
			start := i * 32
			entry := buffer[start : start+32]
			if getUint8(buffer, 0) == 0 {
				done = true
				break
			}
			if !isWantedFile(entry) {
				continue // Ignore garbage
			}

			fileInfo := getFileInfo(context, entry)
			files = append(files, fileInfo)
		}
		if done {
			break
		}
		sector++
	}

	return files
}

//*****************************************************************************
func getSectorForCluster(clusterStartSector int, cluster int) int {
	// Convert cluster to sector. Note: Clusters start at 2!
	return clusterStartSector + (cluster-2)*SECTORS_PER_CLUSTER
}

//*****************************************************************************
func unClusterFuck(buffer []byte) {
	// Sneaky Roland! Swap your partners:
	for i := 0; i < len(buffer); i += 2 {
		x := buffer[i]
		buffer[i] = buffer[i+1]
		buffer[i+1] = x
	}
}

//*****************************************************************************
func isWantedFile(buffer []byte) bool {
	// As far as I know, any file we care about starts with "V" on the file extension.
	var firstCharOfExt = string(buffer[8:9])
	return firstCharOfExt == "V"
}

//*****************************************************************************
/*
func getExtension(buffer []byte) string {
	// As far as I know, any file we care about starts with "V" on the file extension.
	return string(buffer[8:11])
}
*/
//*****************************************************************************
func getFileInfo(context contextType, buffer []byte) fileInfoType {

	var fileInfo fileInfoType

	fileInfo.name = strings.TrimSpace(string(buffer[0:8]))

	fileInfo.ext = string(buffer[8:11])
	fileInfo.attr = getUint8(buffer, 11)
	fileInfo.cluster = getUint16(buffer, 26)
	fileInfo.size = getUint32(buffer, 28)
	fileInfo.isDirectory = (fileInfo.attr == DIRECTORY)
	fileInfo.bootRecord = context.bootRecord
	fileInfo.fat = context.fat
	fileInfo.partition = context.partition

	sector := getSectorForCluster(context.bootRecord.clusterStartSector, fileInfo.cluster)

	if fileInfo.isDirectory {
		fileInfo.dirFiles = getDirectoryEntries(context, sector, true)
	} else {
		fileInfo.clusters = getClusterChain(context.fat, fileInfo.cluster)
	}

	return fileInfo
}

//*****************************************************************************
func getClusterChain(fat []byte, cluster int) []int {
	chain := make([]int, 0, 100000)

	chain = append(chain, cluster)

	index := cluster * 2
	for {
		nextCluster := getUint16(fat, index)
		if nextCluster == 0x0000 {
			fmt.Printf("AVAILABLE CLUSTER\n")
		} else if nextCluster <= 0xffef {
			//fmt.Printf("USED CLUSTER\n")
			chain = append(chain, nextCluster)
			index = nextCluster * 2
		} else if nextCluster > 0xffef && nextCluster <= 0xfff6 {
			fmt.Printf("RESERVED CLUSTER\n")
		} else if nextCluster == 0xfff7 {
			fmt.Printf("BAD CLUSTER\n")
		} else if nextCluster >= 0xfff8 {
			break
		}
	}
	return chain
}

//*****************************************************************************
func getClusterData(clusterStartSector int, cluster int) []byte { //TBD
	sector := getSectorForCluster(clusterStartSector, cluster)
	buffer := make([]byte, SECTOR_SIZE*SECTORS_PER_CLUSTER)

	pos := int64(sector * SECTOR_SIZE)
	vsDrive.Seek(pos, 0)

	_, err := io.ReadFull(vsDrive, buffer)
	if err != nil {
		check(err)
	}
	return buffer
}

//*****************************************************************************
func getSongIndexes(files []fileInfoType) {
	for i := 0; i < len(files); i++ {
		file := &files[i]
		if file.name == "SONGLIST" {
			file.songIndex = getSongIndex(*file)
			partitionSongIndexMap[file.partition] = file.songIndex
		}
	}
}

//*****************************************************************************
func showList(files []fileInfoType) {
	fmt.Printf("Partition   Song\n")
	fmt.Printf("--------------------------\n")
	for i := 0; i < len(files); i++ {
		file := files[i]
		if file.songIndex != nil {
			for key, _ := range file.songIndex {
				//fmt.Printf("%d \"%s\" \"%s\"\n", file.partition, key, value)
				fmt.Printf("%d           \"%s\"\n", file.partition, key)
			}
		}
	}
}

//*****************************************************************************
func readFileComplete(file fileInfoType) []byte {
	// For small files <1MB
	clusterChain := getClusterChain(file.fat, file.cluster)

	buffer := make([]byte, file.size)

	remainingBytes := file.size

	for i := 0; i < len(clusterChain); i++ {
		cluster := clusterChain[i]
		clusterData := getClusterData(file.bootRecord.clusterStartSector, cluster)
		unClusterFuck(clusterData)

		bufferPos := i * len(clusterData)
		if len(clusterData) <= remainingBytes {
			copy(buffer[bufferPos:], clusterData[0:])
			remainingBytes -= len(clusterData)
		} else {
			copy(buffer[bufferPos:bufferPos+remainingBytes], clusterData[0:remainingBytes])
			break
		}
	}
	return buffer
}

//*****************************************************************************
func getSongIndex(file fileInfoType) map[string]string {

	buffer := readFileComplete(file)

	songIndex := make(map[string]string)

	numSongs := getUint16BE(buffer, 0)

	for i := 0; i < numSongs; i++ {

		pos := 6 + i*38
		songNumber := getUint16BE(buffer, pos)
		song := getString(buffer, pos+2, 12)

		songFileName := fmt.Sprintf("SONG%04d", songNumber)

		songIndex[song] = songFileName
	}

	return songIndex
}

//*****************************************************************************
// Utilities
//*****************************************************************************
func readSector(file *os.File, sector int) []byte {

	//fmt.Printf("Reading sector " + strconv.Itoa(int(sector)) + "\n");

	buffer := make([]byte, SECTOR_SIZE)

	pos := int64(sector * SECTOR_SIZE)
	file.Seek(pos, 0)

	_, err := io.ReadFull(file, buffer)
	if err != nil {
		check(err)
	}
	return buffer
}

//*****************************************************************************
func getUint8(buffer []byte, pos int) int {
	return int(buffer[pos])
}

//*****************************************************************************
func getUint16(buffer []byte, pos int) int {
	return int(binary.LittleEndian.Uint16(buffer[pos : pos+2]))
}

//*****************************************************************************
func getUint16BE(buffer []byte, pos int) int {
	return int(binary.BigEndian.Uint16(buffer[pos : pos+2]))
}

//*****************************************************************************
func getUint32(buffer []byte, pos int) int {
	return int(binary.LittleEndian.Uint32(buffer[pos : pos+4]))
}

//*****************************************************************************
func getUint32BE(buffer []byte, pos int) int {
	return int(binary.BigEndian.Uint32(buffer[pos : pos+4]))
}

//*****************************************************************************
func getInt32(buffer []byte, pos int) int {
	return int(binary.LittleEndian.Uint32(buffer[pos : pos+4]))
}

//*****************************************************************************
func getInt32BE(buffer []byte, pos int) int {
	return int(binary.BigEndian.Uint32(buffer[pos : pos+4]))
}

//*****************************************************************************
func getString(buffer []byte, pos int, size int) string {
	return string(buffer[pos : pos+size])
}

//*****************************************************************************
func check(e error) {
	if e != nil {
		//panic(e)
		abort(e.Error())
	}
}

//*****************************************************************************
func showPartitions(partitionSectors []int) {
	fmt.Printf("===========\n")
	fmt.Printf("Partitions:\n")
	fmt.Printf("===========\n")
	fmt.Printf("%v\n", partitionSectors)
}

//*****************************************************************************
func showBootRecord(bootRecord bootRecordType) {
	fmt.Printf("===========\n")
	fmt.Printf("Boot Record:\n")
	fmt.Printf("===========\n")
	fmt.Printf("%v\n", bootRecord)
}

//*****************************************************************************
func showFAT(fat []byte) {
	fmt.Printf("===========\n")
	fmt.Printf("FAT:\n")
	fmt.Printf("===========\n")
	//fmt.Printf("%v\n", fat)
}

//*****************************************************************************
func showFileStructure(files []fileInfoType) {
	fmt.Printf("===============\n")
	fmt.Printf("File structure:\n")
	fmt.Printf("===============\n")
	//fmt.Printf("%v\n", files)
	for i := 0; i < len(files); i++ {
		fmt.Printf("%s.%s\n", strings.TrimSpace(files[i].name), files[i].ext)
	}
}

//*****************************************************************************
func getSongInfo(vsFormat string, file fileInfoType) (string, int, int) {

	buffer := readFileComplete(file)

	var songName string
	var rate byte
	var mode byte

	if getFormat(buffer[0]) == "VS-2480" {
		songName = getString(buffer, 10, 12)
		rate = buffer[22]
		mode = buffer[23]
	} else {
		songName = getString(buffer, 6, 12)
		rate = buffer[18]
		mode = buffer[19]
	}

	return songName, int(rate), int(mode)
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
func getBitDepth(mode int) int {
	switch mode {
	case 0:
		return 16
	case 1:
		return 16
	case 2:
		return 16
	case 3:
		return 16
	case 4:
		return 16
	case 5:
		return 24
	case 6:
		return 16
	case 7:
		return 16
	case 8:
		return 24
	default:
		return 16
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
/*
func findSongListOnPartition(files []fileInfoType, partition int) (fileInfoType, error) {
	var nothing fileInfoType
	for _, file := range files {
		if file.name == "SONGLIST" && file.partition == partition {
			return file, nil
		}
	}
	return nothing, errors.New("Songlist file not found")
}
*/
//*****************************************************************************
func findSongFileOnPartition(files []fileInfoType, friendlyName string, partition int) (fileInfoType, error) {
	fileName := partitionSongIndexMap[partition][friendlyName]
	var nothing fileInfoType
	for _, file := range files {
		if file.name == fileName && file.partition == partition {
			return file, nil
		}
	}
	return nothing, errors.New(fmt.Sprintf("File not found: %s", friendlyName))
}

//*****************************************************************************
func findSongFileByFriendlyName(files []fileInfoType, friendlyName string) (fileInfoType, error) {
	var nothing fileInfoType
	for _, file := range files {
		fileName := partitionSongIndexMap[file.partition][friendlyName]
		if file.name == fileName {
			return file, nil
		}
	}
	return nothing, errors.New(fmt.Sprintf("File not found: %s", friendlyName))
}

//*****************************************************************************
func findFileInDirectory(songDir fileInfoType, fileName string) (fileInfoType, error) {
	var nothing fileInfoType
	for _, file := range songDir.dirFiles {
		if file.name == fileName {
			return file, nil
		}
	}
	return nothing, errors.New("File not found")
}

//*****************************************************************************
func convertVS1880ToReaper(songDirPath fileInfoType, wavDirPath string) {

	//var bitDepth int

	// Create the target directory for the Reaper conversion
	os.MkdirAll(wavDirPath, os.ModePerm)

	// Get song and event file paths.

	songFile, err := findFileInDirectory(songDirPath, "SONG")
	check(err)

	eventFile, err := findFileInDirectory(songDirPath, "EVENTLST")
	check(err)

	// Determine sample rate from song file.

	songName, rate, mode := getSongInfo(vsMachine, songFile)

	modeName := getModeName(mode)

	switch modeName {
	case "MTP":
	case "MT1":
	case "MT2":
	case "M16":
	case "M24": // Actually won't happen with 1680/1880!
	default:
		abort(fmt.Sprintf("%s encodings are currently not supported.", modeName))
	}

	// Open the Reaper project file to create
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
	fmt.Printf("Bit Depth:   %d\n", getBitDepth(mode))
	fmt.Printf("Partition:   %d\n", songDirPath.partition)
	fmt.Printf("\n")

	// Open the event file.

	buffer := readFileComplete(eventFile)

	// Skip past phrases

	numPhrases := getUint16BE(buffer, 16)

	pos := 18 + numPhrases*64

	trackCount := 18
	if vsMachine == "VS-1680" {
		trackCount = 16
	}

	reaperWriter.WriteString("<REAPER_PROJECT 0.1\n")
	reaperWriter.WriteString(fmt.Sprintf("SAMPLERATE %d 1 0\n", int(vsSampleRate)))

	for trackIndex := 1; trackIndex <= trackCount; trackIndex++ {
		for vtrackIndex := 1; vtrackIndex <= 16; vtrackIndex++ {

			pos += 16

			numEvents := getUint16BE(buffer, pos)

			pos += 2

			if numEvents == 0 {
				continue
			}

			trackName := fmt.Sprintf("Track%d-%d", trackIndex, vtrackIndex)

			reaperWriter.WriteString("<TRACK\n")
			reaperWriter.WriteString(fmt.Sprintf("NAME \"%s\"\n", trackName))

			for i := 0; i < numEvents; i++ {

				eventStart := getUint32BE(buffer, pos)
				eventEnd := getUint32BE(buffer, pos+4)
				eventOffset := getUint32BE(buffer, pos+8)

				sampleStart := float64(eventStart * 16)
				sampleEnd := float64(eventEnd * 16)
				sampleOffset := float64(eventOffset * 16)

				startSecs := sampleStart / vsSampleRate
				endSecs := sampleEnd / vsSampleRate
				offsetSecs := sampleOffset / vsSampleRate

				eventLengthSecs := endSecs - startSecs

				takeFileId := getUint16BE(buffer, pos+20)

				takeFileName := fmt.Sprintf("TAKE%04X", takeFileId)

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
				reaperWriter.WriteString(">\n")

				message := fmt.Sprintf("Converting %s to %s", takeFileName, wavFilePath)
				takeFile, err := findFileInDirectory(songDirPath, takeFileName)
				check(err)
				switch modeName {
				case "MTP":
					convertMTP(takeFile, wavFilePath, int(vsSampleRate), message)
				case "MT1":
					convertMT1(takeFile, wavFilePath, int(vsSampleRate), message)
				case "MT2":
					convertMT2(takeFile, wavFilePath, int(vsSampleRate), message)
				case "M16":
					convertM16(takeFile, wavFilePath, int(vsSampleRate), message)
				case "M24":
					convertM24(takeFile, wavFilePath, int(vsSampleRate), message) // Won't happen with 1680/1880
				default:
				}
				fmt.Printf("\n")

				pos += 64
			}
			reaperWriter.WriteString(">\n")
		}
	}
	reaperWriter.WriteString(">\n")
	fmt.Printf("\nCreated Reaper project file: %s\n", reaperProject)
}

//*****************************************************************************
func convertMTP(file fileInfoType, outFileName string, sampleRate int, message string) {
	cluster := file.cluster
	clusterChain := getClusterChain(file.fat, cluster)

	f, err := os.Create(outFileName)
	check(err)
	defer f.Close()

	wavWriter := bufio.NewWriter(f)

	fileSize := file.size
	totalBlocks := int(fileSize / 16)

	// Write the WAV header
	numSamples := totalBlocks * 16
	writeWavHeader(wavWriter, numSamples, sampleRate, 24, 1)

	in := make([]uint8, 16)
	out := make([]int, 16)
	d0 := 0

	for i := 0; i < len(clusterChain); i++ {
		cluster := clusterChain[i]
		clusterData := getClusterData(file.bootRecord.clusterStartSector, cluster)
		unClusterFuck(clusterData)

		numBlocks := len(clusterData) / 16

		var percentComplete float32 = float32(i+1) / float32(len(clusterChain)) * 100.0

		fmt.Printf("\r%s: %3d%%", message, int(percentComplete))

		for j := 0; j < numBlocks; j++ {
			clusterPos := j * 16

			copy(in, clusterData[clusterPos:clusterPos+16])

			decode.DecodeMTP(d0, in, out)
			writeWavSamples24(wavWriter, out)
			d0 = out[15]
		}
		wavWriter.Flush()
		f.Sync()
	}
}

//*****************************************************************************
func convertMT1(file fileInfoType, outFileName string, sampleRate int, message string) {
	cluster := file.cluster
	clusterChain := getClusterChain(file.fat, cluster)

	f, err := os.Create(outFileName)
	check(err)
	defer f.Close()

	wavWriter := bufio.NewWriter(f)

	fileSize := file.size
	totalBlocks := int(fileSize / 16)

	// Write the WAV header
	numSamples := totalBlocks * 16
	writeWavHeader(wavWriter, numSamples, sampleRate, 16, 1)

	in := make([]uint8, 16)
	out := make([]int, 16)
	d0 := 0

	for i := 0; i < len(clusterChain); i++ {
		cluster := clusterChain[i]
		clusterData := getClusterData(file.bootRecord.clusterStartSector, cluster)
		unClusterFuck(clusterData)

		numBlocks := len(clusterData) / 16

		var percentComplete float32 = float32(i+1) / float32(len(clusterChain)) * 100.0

		fmt.Printf("\r%s: %3d%%", message, int(percentComplete))

		for j := 0; j < numBlocks; j++ {
			clusterPos := j * 16

			copy(in, clusterData[clusterPos:clusterPos+16])

			decode.DecodeMT1(d0, in, out)
			writeWavSamples16(wavWriter, out)
			d0 = out[15]
		}
		wavWriter.Flush()
		f.Sync()
	}
}

//*****************************************************************************
func convertMT2(file fileInfoType, outFileName string, sampleRate int, message string) {
	cluster := file.cluster
	clusterChain := getClusterChain(file.fat, cluster)

	f, err := os.Create(outFileName)
	check(err)
	defer f.Close()

	wavWriter := bufio.NewWriter(f)

	fileSize := file.size
	totalBlocks := int(fileSize / 12)

	// Write the WAV header
	numSamples := totalBlocks * 16
	writeWavHeader(wavWriter, numSamples, sampleRate, 16, 1)

	in := make([]uint8, 12)
	out := make([]int, 16)
	d0 := 0

	for i := 0; i < len(clusterChain); i++ {
		cluster := clusterChain[i]
		clusterData := getClusterData(file.bootRecord.clusterStartSector, cluster)
		unClusterFuck(clusterData)

		numBlocks := len(clusterData) / 12

		var percentComplete float32 = float32(i+1) / float32(len(clusterChain)) * 100.0

		fmt.Printf("\r%s: %3d%%", message, int(percentComplete))

		for j := 0; j < numBlocks; j++ {
			clusterPos := j * 12

			copy(in, clusterData[clusterPos:clusterPos+12])

			decode.DecodeMT2(d0, in, out)
			writeWavSamples16(wavWriter, out)
			d0 = out[15]
		}
		wavWriter.Flush()
		f.Sync()
	}
}

//*****************************************************************************
func convertM16(file fileInfoType, outFileName string, sampleRate int, message string) {
	cluster := file.cluster
	clusterChain := getClusterChain(file.fat, cluster)

	f, err := os.Create(outFileName)
	check(err)
	defer f.Close()

	wavWriter := bufio.NewWriter(f)

	fileSize := file.size
	totalBlocks := int(fileSize / 32)

	// Write the WAV header
	numSamples := totalBlocks * 16
	writeWavHeader(wavWriter, numSamples, sampleRate, 16, 1)

	in := make([]uint8, 32)
	out := make([]int, 16)
	d0 := 0

	for i := 0; i < len(clusterChain); i++ {
		cluster := clusterChain[i]
		clusterData := getClusterData(file.bootRecord.clusterStartSector, cluster)
		unClusterFuck(clusterData)

		numBlocks := len(clusterData) / 32

		var percentComplete float32 = float32(i+1) / float32(len(clusterChain)) * 100.0

		fmt.Printf("\r%s: %3d%%", message, int(percentComplete))

		for j := 0; j < numBlocks; j++ {
			clusterPos := j * 32

			copy(in, clusterData[clusterPos:clusterPos+32])

			decode.DecodeM16(d0, in, out)
			writeWavSamples16(wavWriter, out)
			d0 = out[15]
		}
		wavWriter.Flush()
		f.Sync()
	}
}

//*****************************************************************************
func convertM24(file fileInfoType, outFileName string, sampleRate int, message string) {
	cluster := file.cluster
	clusterChain := getClusterChain(file.fat, cluster)

	f, err := os.Create(outFileName)
	check(err)
	defer f.Close()

	wavWriter := bufio.NewWriter(f)

	fileSize := file.size
	totalBlocks := int(fileSize / 48)

	// Write the WAV header
	numSamples := totalBlocks * 16
	writeWavHeader(wavWriter, numSamples, sampleRate, 24, 1)

	in := make([]uint8, 48)
	out := make([]int, 16)
	d0 := 0

	for i := 0; i < len(clusterChain); i++ {
		cluster := clusterChain[i]
		clusterData := getClusterData(file.bootRecord.clusterStartSector, cluster)
		unClusterFuck(clusterData)

		numBlocks := len(clusterData) / 48

		var percentComplete float32 = float32(i+1) / float32(len(clusterChain)) * 100.0

		fmt.Printf("\r%s: %3d%%", message, int(percentComplete))

		for j := 0; j < numBlocks; j++ {
			clusterPos := j * 48

			copy(in, clusterData[clusterPos:clusterPos+48])

			decode.DecodeM24(d0, in, out)
			writeWavSamples24(wavWriter, out)
			d0 = out[15]
		}
		wavWriter.Flush()
		f.Sync()
	}
}

//*****************************************************************************
func writeWavHeader(w *bufio.Writer, numSamples int, sampleRate int, bitDepth int, numChannels int) {

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
}

//*****************************************************************************
func writeWavSamples16(w *bufio.Writer, samples []int) {
	// Write it to the WAV (little-endian)
	for i := 0; i < 16; i++ {
		sample := []byte{
			BYTE_0(samples[i]),
			BYTE_1(samples[i]),
		}
		w.Write(sample)
	}
}

//*****************************************************************************
func writeWavSamples24(w *bufio.Writer, samples []int) {
	// Write it to the WAV (little-endian)
	for i := 0; i < 16; i++ {
		sample := []byte{
			BYTE_0(samples[i]),
			BYTE_1(samples[i]),
			BYTE_2(samples[i]),
		}
		w.Write(sample)
	}
}

//*****************************************************************************
func BYTE_0(xx int) byte { return byte(xx & 0xff) }
func BYTE_1(xx int) byte { return byte((xx >> 8) & 0xff) }
func BYTE_2(xx int) byte { return byte((xx >> 16) & 0xff) }
func BYTE_3(xx int) byte { return byte((xx >> 24) & 0xff) }

//*****************************************************************************
func abort(message string) {
	fmt.Printf("%s\n\n", message)
	os.Exit(0)
}
