/*
 * Copyright 2016 Analog Devices, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Radio; see the file LICENSE.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street,
 * Boston, MA 02110-1301, USA.
 */

#include "adc_sample_conv.hpp"
#include "osc_adc.h"

using namespace gr;
using namespace adiscope;

adc_sample_conv::adc_sample_conv(int nconnections,
				 std::shared_ptr<M2kAdc> adc,
				 bool inverse) :
	gr::sync_block("adc_sample_conv",
			gr::io_signature::make(nconnections, nconnections, sizeof(float)),
			gr::io_signature::make(nconnections, nconnections, sizeof(float))),
	d_nconnections(nconnections),
	inverse(inverse),
	m2k_adc(adc)
{
	for (int i = 0; i < d_nconnections; i++) {
		d_correction_gains.push_back(1.0);
		d_filter_compensations.push_back(1.0);
		d_offsets.push_back(0.0);
		d_hardware_gains.push_back(0.02);
	}
}

adc_sample_conv::~adc_sample_conv()
{
}

float adc_sample_conv::convSampleToVolts(float sample, float correctionGain,
	float filterCompensation, float offset, float hw_gain)
{
	// TO DO: explain this formula
	return ((sample * 0.78) / ((1 << 11) * 1.3 * hw_gain) *
		correctionGain * filterCompensation) + offset;
}

float adc_sample_conv::convVoltsToSample(float voltage, float correctionGain,
	float filterCompensation, float offset, float hw_gain)
{
	// TO DO: explain this formula
	return ((voltage - offset) / (correctionGain * filterCompensation) *
			(2048 * 1.3 * hw_gain) / 0.78);
}

int adc_sample_conv::work(int noutput_items,
		gr_vector_const_void_star &input_items,
		gr_vector_void_star &output_items)
{
	updateCorrectionGain();
	for (unsigned int i = 0; i < input_items.size(); i++) {
		const float* in = static_cast<const float *>(input_items[i]);
		float *out = static_cast<float *>(output_items[i]);

		if (inverse)
			for (unsigned int j = 0; j < noutput_items; j++)
				out[j] = convVoltsToSample(in[j],
						d_correction_gains[i],
						d_filter_compensations[i],
						d_offsets[i],
						d_hardware_gains[i]);
		else
			for (unsigned int j = 0; j < noutput_items; j++)
				out[j] = convSampleToVolts(in[j],
					d_correction_gains[i],
					d_filter_compensations[i],
					d_offsets[i],
					d_hardware_gains[i]);
	}

	return noutput_items;
}

void adc_sample_conv::updateCorrectionGain()
{
        if(m2k_adc) {
            setCorrectionGain(0, m2k_adc->chnCorrectionGain(0));
//            setCorrectionGain(1, m2k_adc->chnCorrectionGain(1));
            setFilterCompensation(0, m2k_adc->compTable(m2k_adc->sampleRate()));
//            setFilterCompensation(1, m2k_adc->compTable(m2k_adc->sampleRate()));
            setHardwareGain(0, m2k_adc->gainAt(m2k_adc->chnHwGainMode(0)));
//            setHardwareGain(1, m2k_adc->gainAt(m2k_adc->chnHwGainMode(1)));
        }
}

void adc_sample_conv::setCorrectionGain(int connection, float gain)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_correction_gains[connection] != gain) {
		gr::thread::scoped_lock lock(d_setlock);
		d_correction_gains[connection] = gain;
	}
}

float adc_sample_conv::correctionGain(int connection)
{
	if (connection >= 0 && connection < d_nconnections)
		return d_correction_gains[connection];

	return 0.0;
}

void adc_sample_conv::setFilterCompensation(int connection, float val)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_filter_compensations[connection] != val) {
		gr::thread::scoped_lock lock(d_setlock);
		d_filter_compensations[connection] = val;
	}
}

float adc_sample_conv::filterCompensation(int connection)
{
	if (connection >= 0 && connection < d_nconnections)
		return d_filter_compensations[connection];

	return 0.0;
}

void adc_sample_conv::setOffset(int connection, float offset)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_offsets[connection] != offset) {
		gr::thread::scoped_lock lock(d_setlock);
		d_offsets[connection] = offset;
	}
}

float adc_sample_conv::offset(int connection) const
{
	if (connection >= 0 && connection < d_nconnections)
		return d_offsets[connection];

	return 0;
}

void adc_sample_conv::setHardwareGain(int connection, float gain)
{
	if (connection < 0 || connection >= d_nconnections)
		return;

	if (d_hardware_gains[connection] != gain) {
		gr::thread::scoped_lock lock(d_setlock);
		d_hardware_gains[connection] = gain;
	}
}

float adc_sample_conv::hardwareGain(int connection) const
{
	if (connection >= 0 && connection < d_nconnections)
		return d_hardware_gains[connection];

	return 0.0;
}
