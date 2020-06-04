/*
  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  It is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

void writeWavHeader(FILE *fout, int numSamples, int sampleRate, int bitDepth, int numChannels);

void writeWavSamples24(FILE *fout, int samples[]);

void writeWavSamples24as16(FILE *fout, int samples[]);

void writeWavSamples16(FILE *fout, int samples[]);

void writeWavSamples16as24(FILE *fout, int samples[]);

void writeWavSamples16Stereo(FILE *fout, int samples[]);

void writeWavSamples16as24Stereo(FILE *fout, int samples[]);

