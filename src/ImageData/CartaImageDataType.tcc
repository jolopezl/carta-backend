/* This file is part of the CARTA Image Viewer: https://github.com/CARTAvis/carta-backend
   Copyright 2018, 2019, 2020, 2021 Academia Sinica Institute of Astronomy and Astrophysics (ASIAA),
   Associated Universities, Inc. (AUI) and the Inter-University Institute for Data Intensive Astronomy (IDIA)
   SPDX-License-Identifier: GPL-3.0-or-later
*/

#ifndef CARTA_BACKEND_IMAGEDATA_CARTAIMAGEDATATYPE_TCC_
#define CARTA_BACKEND_IMAGEDATA_CARTAIMAGEDATATYPE_TCC_

#include <casacore/casa/BasicSL/Constants.h>
#include <casacore/casa/OS/File.h>
#include <casacore/casa/Quanta/UnitMap.h>
#include <casacore/coordinates/Coordinates/DirectionCoordinate.h>
#include <casacore/coordinates/Coordinates/FITSCoordinateUtil.h>
#include <casacore/coordinates/Coordinates/SpectralCoordinate.h>
#include <casacore/images/Images/ImageFITSConverter.h>
#include <casacore/lattices/Lattices/MaskedLatticeIterator.h>
#include <casacore/measures/Measures/MDirection.h>
#include <casacore/measures/Measures/Stokes.h>
#include <casacore/tables/DataMan/TiledFileAccess.h>

#include <images/Images/SubImage.h>
#include <wcslib/fitshdr.h>
#include <wcslib/wcs.h>
#include <wcslib/wcsfix.h>
#include <wcslib/wcshdr.h>
#include <wcslib/wcsmath.h>

#include "Util/Casacore.h"

using namespace carta;

template <class T>
CartaImageDataType<T>::CartaImageDataType(const std::string& filename) : casacore::ImageInterface<float>(), _filename(filename) {
    switch (CasacoreImageType(filename)) {
        case casacore::ImageOpener::AIPSPP:
            _image.reset(new casacore::PagedImage<T>(filename));
            break;
        default:
            spdlog::error("Unknown image type!");
            break;
    }
    setCoordinateInfo(_image->coordinates());
}

template <class T>
CartaImageDataType<T>::CartaImageDataType(const CartaImageDataType& other)
    : casacore::ImageInterface<float>(other), _filename(other._filename) {
    switch (CasacoreImageType(other._filename)) {
        case casacore::ImageOpener::AIPSPP:
            _image.reset(new casacore::PagedImage<T>(other._filename));
            break;
        default:
            spdlog::error("Unknown image type!");
            break;
    }
    setCoordinateInfo(_image->coordinates());
}

template <class T>
CartaImageDataType<T>::~CartaImageDataType() {}

// Image interface

template <class T>
casacore::String CartaImageDataType<T>::imageType() const {
    return "CartaImageDataType";
}

template <class T>
casacore::String CartaImageDataType<T>::name(bool stripPath) const {
    return _image->name();
}

template <class T>
casacore::IPosition CartaImageDataType<T>::shape() const {
    return _image->shape();
}

template <class T>
casacore::Bool CartaImageDataType<T>::ok() const {
    return _image->ok();
}

template <class T>
casacore::DataType CartaImageDataType<T>::dataType() const {
    return casacore::DataType::TpFloat;
}

template <class T>
casacore::Bool CartaImageDataType<T>::doGetSlice(casacore::Array<float>& buffer, const casacore::Slicer& section) {
    if constexpr (std::is_same_v<T, casacore::Int>) {
        casacore::SubImage<casacore::Int> sub_image(*_image, section);
        casacore::RO_MaskedLatticeIterator<casacore::Int> lattice_iter(sub_image);
        for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
            casacore::Array<float> cursor_data = (const casacore::Array<float>&)lattice_iter.cursor();
            casacore::IPosition cursor_shape(lattice_iter.cursorShape());
            casacore::IPosition cursor_position(lattice_iter.position());
            casacore::Slicer cursor_slicer(cursor_position, cursor_shape);
            buffer(cursor_slicer) = cursor_data;
        }
    } else if constexpr (std::is_same_v<T, casacore::Bool>) {
        casacore::SubImage<casacore::Bool> sub_image(*_image, section);
        casacore::RO_MaskedLatticeIterator<casacore::Bool> lattice_iter(sub_image);
        for (lattice_iter.reset(); !lattice_iter.atEnd(); ++lattice_iter) {
            casacore::Array<casacore::Bool> cursor_data = lattice_iter.cursor();
            casacore::IPosition cursor_shape(lattice_iter.cursorShape());
            casacore::IPosition cursor_position(lattice_iter.position());
            casacore::Slicer cursor_slicer(cursor_position, cursor_shape);

            // represent the bool type as the float type
            casacore::Array<float> cursor_data_float(cursor_shape);
            auto* bool_ptr = cursor_data.data();
            auto* float_ptr = cursor_data_float.data();
            for (casacore::uInt i = 0; i < cursor_data.size(); ++i) {
                if (bool_ptr[i]) {
                    float_ptr[i] = 1.0;
                } else {
                    float_ptr[i] = 0.0;
                }
            }
            buffer(cursor_slicer) = cursor_data_float;
        }
    } else {
        spdlog::error("Image data type is not supported!");
        return false;
    }
    return true;
}

template <class T>
void CartaImageDataType<T>::doPutSlice(
    const casacore::Array<float>& buffer, const casacore::IPosition& where, const casacore::IPosition& stride) {
    throw(casacore::AipsError("CartaImageDataType::doPutSlice - image is not writable"));
}

template <class T>
const casacore::LatticeRegion* CartaImageDataType<T>::getRegionPtr() const {
    return nullptr;
}

template <class T>
casacore::ImageInterface<float>* CartaImageDataType<T>::cloneII() const {
    return new CartaImageDataType(*this);
}

template <class T>
void CartaImageDataType<T>::resize(const casacore::TiledShape& newShape) {
    throw(casacore::AipsError("CartaImageDataType::resize - image is not writable"));
}

template <class T>
casacore::uInt CartaImageDataType<T>::advisedMaxPixels() const {
    return _image->advisedMaxPixels();
}

template <class T>
casacore::IPosition CartaImageDataType<T>::doNiceCursorShape(casacore::uInt) const {
    return _image->doNiceCursorShape(_image->advisedMaxPixels());
}

template <class T>
casacore::Bool CartaImageDataType<T>::isMasked() const {
    return _image->isMasked();
}

template <class T>
casacore::Bool CartaImageDataType<T>::hasPixelMask() const {
    return _image->hasPixelMask();
}

template <class T>
const casacore::Lattice<bool>& CartaImageDataType<T>::pixelMask() const {
    return _image->pixelMask();
}

template <class T>
casacore::Lattice<bool>& CartaImageDataType<T>::pixelMask() {
    return _image->pixelMask();
}

template <class T>
casacore::Bool CartaImageDataType<T>::doGetMaskSlice(casacore::Array<bool>& buffer, const casacore::Slicer& section) {
    return _image->doGetMaskSlice(buffer, section);
}

#endif // CARTA_BACKEND_IMAGEDATA_CARTAIMAGEDATATYPE_TCC_
