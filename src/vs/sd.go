package main

import (
	"fmt"
	//"log"
	"encoding/binary"
	"os"
	//"errors"
	"strconv"
	"strings"
	//"io/ioutil"
	"vs/vsio"
)

const SECTOR_SIZE = 512
const SECTORS_PER_CLUSTER = 64
const SECTORS_PER_FAT = 256
const DIRECTORY = 16

var targetDir = "c:/testout"

var file *os.File

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
}

type contextType struct {
	file       *os.File
	bootRecord bootRecordType
	fat        []byte
}

//*****************************************************************************
func main_old() {

	// Find the attached VS drive, if any
	file = findRolandDrive()

	// Get the start sectors for each partition
	partitionSectors := getPartitionSectors(file)
	//showPartitions(partitionSectors);

	os.MkdirAll(targetDir, os.ModePerm)

	for i := 0; i < 8; i++ {
		exportPartition(partitionSectors, i)
	}
}

//*****************************************************************************
func main() {

	args := os.Args[1:]

	argCount := len(args)

	if argCount == 0 {
		//fmt.Printf("Invalid number of arguments.")
		//os.Exit(1)
		help()
	}

	command := args[0]

	if "help" == command {
		help()
	} else if "list" == command {
		vsio.List()
	} else if "get" == command {
		if argCount == 2 {
			songName := args[1]
			targetDir := fmt.Sprintf("./%s", songName)
			vsio.ConvertFirst(songName, targetDir)
		} else if argCount == 3 {
			songName := args[1]
			targetDir := fmt.Sprintf("%s/%s", args[2], songName)
			vsio.ConvertFirst(songName, targetDir)
		} else {
			fmt.Printf("Invalid number of arguments.")
			os.Exit(1)
		}
	} else if "getp" == command {
		if argCount == 3 {
			partition, err := strconv.Atoi(args[1])
			if err != nil {
				fmt.Printf("Invalid partition: %s\n", args[1])
				os.Exit(1)
			}
			songName := args[2]
			targetDir := fmt.Sprintf("./%s-%d", songName, partition)
			vsio.Convert(songName, partition, targetDir)
		} else if argCount == 4 {
			partition, err := strconv.Atoi(args[1])
			if err != nil {
				fmt.Printf("Invalid partition: %s\n", args[1])
				os.Exit(1)
			}
			songName := args[2]
			targetDir := fmt.Sprintf("%s/%s-%d", args[3], songName, partition)
			vsio.Convert(songName, partition, targetDir)
		} else {
			fmt.Printf("Invalid number of arguments.")
			os.Exit(1)
		}
	} else {
		fmt.Printf("Invalid command: %s\n", args[0])
		os.Exit(1)
	}
}

//*****************************************************************************
func help() {
	fmt.Printf("\n")
	fmt.Printf("Roland VS to Reaper Project Converter 1.01\n")
	fmt.Printf("------------------------------------------\n")
	fmt.Printf("\n")
	fmt.Printf("Copyright 2017 Randy Gordon (randy@integrand.com)\n")
	fmt.Printf("\n")
	fmt.Printf("Notes: 1) Must be run with administrator privileges.\n")
	fmt.Printf("       2) Supported VS formats: VS-1880 and VS-1680.\n")
	fmt.Printf("       3) Supported RDAC modes: MTP, MT2, MT1, M16, M24.\n")
	fmt.Printf("\n")
	fmt.Printf("Usage examples:\n")
	fmt.Printf("\n")
	fmt.Printf("vs help                    - Show help info\n")
	fmt.Printf("vs list                    - Show all songs on all partitions\n")
	fmt.Printf("vs get \"My Song\"           - Find the first occurrence of song and convert to current directory\n")
	fmt.Printf("vs get \"My Song\" c:/foo    - Find the first occurrence of song and convert to directory c:/foo\n")
	fmt.Printf("vs getp 5 \"My Song\"        - Find song on partition 5 and convert to current directory\n")
	fmt.Printf("vs getp 5 \"My Song\" c:/foo - Find song on partition 5 and convert to directory c:/foo\n")
	fmt.Printf("\n")
	os.Exit(0)
}

//*****************************************************************************
func exportPartition(partitionSectors []int, partition int) {

	fmt.Printf("Exporting Partition %d\n", partition)
	fmt.Printf("*********************\n")

	// Read the boot record for the partition
	bootRecord := getBootRecord(file, partitionSectors[partition])
	//showBootRecord(bootRecord);

	context := newContext(file, bootRecord)

	fat := getFAT(context)
	//showFAT(fat);

	context.fat = fat

	// Get the root directory, and recurse to all (only one level!) subdirectories.
	files := getDirectoryEntries(context, bootRecord.rootStartSector, false)
	//showFileStructure(files);

	partitionDir := targetDir + "/Partition" + strconv.Itoa(partition)

	os.MkdirAll(partitionDir, os.ModePerm)

	// Copy all the gathered file and directory data into a normal Windows filesystem.
	writeFiles(context, partitionDir, files)
}

//*****************************************************************************
func newContext(file *os.File, bootRecord bootRecordType) contextType {
	var context contextType
	context.file = file
	context.bootRecord = bootRecord
	return context
}

//*****************************************************************************
func findRolandDrive() *os.File {
	// Find the Roland VS drive - look at up to 10 drives
	for i := 0; i < 10; i++ {
		drive := "\\\\.\\PhysicalDrive" + strconv.Itoa(i)
		file, err := os.Open(drive)
		if err != nil {
			continue
		}
		if isRolandDrive(file) {
			fmt.Printf("Found Roland VS drive at: %s\n\n", drive)
			return file
		}
	}

	fmt.Printf("%s\n", "Unable to locate an attached Roland VS drive")
	os.Exit(0)

	return nil
}

//*****************************************************************************
func isRolandDrive(file *os.File) bool {
	// Check for the signature
	buffer := readSector(file, 63)
	fmt.Printf("drive = %s\n", string(buffer[3:9]))
	return "Roland" == string(buffer[3:9])
}

//*****************************************************************************
func getPartitionSectors(file *os.File) []int {

	partitionSectors := make([]int, 8)
	partitionOffset := 446

	buffer := readSector(file, 0)

	//fmt.Printf("read bytes: %x\n", buffer)

	for i := 0; i < 4; i++ {
		firstSector := getInt32(buffer, partitionOffset+8)
		//sectorCount := binary.LittleEndian.Uint32(buffer[partitionOffset+12:])
		partitionSectors[i] = firstSector
		partitionOffset += 16
	}

	// Extrapolate extra four partitions
	delta := partitionSectors[1] - partitionSectors[0]

	nextPartitionSector := partitionSectors[3] + delta
	for i := 0; i < 4; i++ {
		partitionSectors[i+4] = nextPartitionSector
		nextPartitionSector += delta
	}

	return partitionSectors
}

//*****************************************************************************
func getBootRecord(file *os.File, sector int) bootRecordType {

	buffer := readSector(file, sector)

	var bootRecord bootRecordType

	bootRecord.formattingOS = string(buffer[3:9])
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

	return bootRecord
}

//*****************************************************************************
func getFAT(context contextType) []byte {
	buffer := make([]byte, SECTOR_SIZE*SECTORS_PER_FAT)

	sector := context.bootRecord.fatStartSector

	pos := int64(sector * SECTOR_SIZE)
	context.file.Seek(pos, 0)

	_, err := context.file.Read(buffer)
	if err != nil {
		check(err)
	}
	return buffer
}

//*****************************************************************************
func getDirectoryEntries(context contextType, sector int, isClusterFucked bool) []fileInfoType {
	done := false
	files := make([]fileInfoType, 0, 1000)

	for {
		buffer := readSector(file, sector)

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
func getSectorForCluster(context contextType, cluster int) int {
	// Convert cluster to sector. Note: Clusters start at 2 - historical anomaly!
	return context.bootRecord.clusterStartSector + (cluster-2)*SECTORS_PER_CLUSTER
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
	var fistCharOfExt = string(buffer[8:9])
	return fistCharOfExt == "V"
}

//*****************************************************************************
func getFileInfo(context contextType, buffer []byte) fileInfoType {

	var fileInfo fileInfoType

	fileInfo.name = string(buffer[0:8])
	fileInfo.ext = string(buffer[8:11])
	fileInfo.attr = getUint8(buffer, 11)
	fileInfo.cluster = getUint16(buffer, 26)
	fileInfo.size = getUint32(buffer, 28)
	fileInfo.isDirectory = (fileInfo.attr == DIRECTORY)

	sector := getSectorForCluster(context, fileInfo.cluster)

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
		if nextCluster <= 0xffef {
			chain = append(chain, nextCluster)
		} else if nextCluster >= 0xfff8 {
			break
		}
		index += 2
	}
	return chain
}

//*****************************************************************************
func getClusterData(context contextType, cluster int) []byte { //TBD
	sector := getSectorForCluster(context, cluster)
	buffer := make([]byte, SECTOR_SIZE*SECTORS_PER_CLUSTER)

	pos := int64(sector * SECTOR_SIZE)
	file.Seek(pos, 0)

	_, err := file.Read(buffer)
	if err != nil {
		check(err)
	}
	return buffer
}

//*****************************************************************************
func writeFiles(context contextType, targetDir string, files []fileInfoType) {
	// Write out the unclusterfucked results to the target filesystem.
	for i := 0; i < len(files); i++ {
		file := files[i]
		targetFileName := targetDir + "/" + strings.TrimSpace(file.name) + "." + file.ext
		if file.attr == DIRECTORY {
			//if (!os..existsSync(targetFileName)){
			//    fs.mkdirSync(targetFileName);
			//}
			fmt.Printf("Creating dir:  %s\n", targetFileName)
			os.MkdirAll(targetFileName, os.ModePerm)
			writeFiles(context, targetFileName, file.dirFiles)
		} else {
			fmt.Printf("Creating file: %s (%d)\n", targetFileName, file.size)
			writeFileData(context, targetFileName, file.cluster, file.size)
		}
	}
}

//*****************************************************************************
func writeFileData(context contextType, targetFileName string, cluster int, size int) {
	clusterChain := getClusterChain(context.fat, cluster)
	remainingBytes := size

	f, err := os.Create(targetFileName)
	check(err)
	defer f.Close()

	for i := 0; i < len(clusterChain); i++ {
		cluster := clusterChain[i]
		clusterData := getClusterData(context, cluster)
		unClusterFuck(clusterData)

		if len(clusterData) <= remainingBytes {
			_, err := f.Write(clusterData)
			check(err)
			//fmt.Printf("wrote %d bytes\n", count)
			remainingBytes -= len(clusterData)
		} else {
			_, err := f.Write(clusterData[0:remainingBytes])
			check(err)
			//fmt.Printf("wrote final %d bytes\n", count)
			break
		}
	}

	f.Sync()
}

//*****************************************************************************
// Utilities
//*****************************************************************************
func readSector(file *os.File, sector int) []byte {

	//fmt.Printf("Reading sector " + strconv.Itoa(int(sector)) + "\n");

	buffer := make([]byte, SECTOR_SIZE)

	pos := int64(sector * SECTOR_SIZE)
	file.Seek(pos, 0)

	_, err := file.Read(buffer)
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
	return int(binary.LittleEndian.Uint16(buffer[pos:]))
}

//*****************************************************************************
func getUint32(buffer []byte, pos int) int {
	return int(binary.LittleEndian.Uint32(buffer[pos:]))
}

//*****************************************************************************
func getInt32(buffer []byte, pos int) int {
	return int(binary.LittleEndian.Uint32(buffer[pos:]))
}

//*****************************************************************************
func check(e error) {
	if e != nil {
		panic(e)
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
