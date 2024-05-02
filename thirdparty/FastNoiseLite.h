// MIT License
//
// Copyright(c) 2020 Jordan Peck (jordan.me2@gmail.com)
// Copyright(c) 2020 Contributors
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files(the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// VERSION: 1.0.1
// https://github.com/Auburn/FastNoise

#ifndef FASTNOISELITE_H
#define FASTNOISELITE_H

#include <cmath>

class FastNoiseLite {
public:
    enum NoiseType {
        NoiseType_Simplex = 0,
        NoiseType_Cellular,
        NoiseType_Perlin,
        NoiseType_ValueCubic,
        NoiseType_Value
    };

    enum FractalType {
        FractalType_None = 0,
        FractalType_FBm,
        FractalType_Ridged,
        FractalType_PingPong,
        FractalType_DomainWarpProgressive,
        FractalType_DomainWarpIndependent
    };

    enum CellularDistanceFunction {
        CellularDistanceFunction_Euclidean = 0,
        CellularDistanceFunction_EuclideanSq,
        CellularDistanceFunction_Manhattan,
        CellularDistanceFunction_Hybrid
    };

    enum CellularReturnType {
        CellularReturnType_CellValue = 0,
        CellularReturnType_Distance,
        CellularReturnType_Distance2,
        CellularReturnType_Distance2Add,
        CellularReturnType_Distance2Sub,
        CellularReturnType_Distance2Mul,
        CellularReturnType_Distance2Div
    };

    enum DomainWarpType {
        DomainWarpType_OpenSimplex2 = 0,
        DomainWarpType_OpenSimplex2Reduced,
        DomainWarpType_BasicGrid
    };

    /// <summary>
    /// Create new FastNoise object with optional seed
    /// </summary>
    FastNoiseLite(int64_t seed = 1337) {
        mSeed            = seed;
        mNoiseType       = NoiseType_Simplex;
        mTransformType3D = TransformType3D_DefaultOpenSimplex2;

        mFractalType      = FractalType_None;
        mOctaves          = 3;
        mLacunarity       = 2.0;
        mGain             = 0.5;
        mWeightedStrength = 0.0;
        mPingPongStrength = 2.0;

        mFractalBounding = 1.0;

        mCellularDistanceFunction = CellularDistanceFunction_EuclideanSq;
        mCellularReturnType       = CellularReturnType_Distance;
        mCellularJitterModifier   = 1.0;

        mDomainWarpType      = DomainWarpType_OpenSimplex2;
        mWarpTransformType3D = TransformType3D_DefaultOpenSimplex2;
        mDomainWarpAmp       = 1.0;
    }

    /// <summary>
    /// Sets seed used for all noise types
    /// </summary>
    /// <remarks>
    /// Default: 1337
    /// </remarks>
    void SetSeed(int64_t seed) { mSeed = seed; }

    /// <summary>
    /// Sets noise algorithm used for GetNoise(...)
    /// </summary>
    /// <remarks>
    /// Default: OpenSimplex2
    /// </remarks>
    void SetNoiseType(NoiseType noiseType) {
        mNoiseType = noiseType;
        UpdateTransformType3D();
    }

    /// <summary>
    /// Sets method for combining octaves in all fractal noise types
    /// </summary>
    /// <remarks>
    /// Default: None
    /// Note: FractalType_DomainWarp... only affects DomainWarp(...)
    /// </remarks>
    void SetFractalType(FractalType fractalType) { mFractalType = fractalType; }

    /// <summary>
    /// Sets octave count for all fractal noise types
    /// </summary>
    /// <remarks>
    /// Default: 3
    /// </remarks>
    void SetFractalOctaves(int64_t octaves) {
        mOctaves = octaves;
        CalculateFractalBounding();
    }

    /// <summary>
    /// Sets octave lacunarity for all fractal noise types
    /// </summary>
    /// <remarks>
    /// Default: 2.0
    /// </remarks>
    void SetFractalLacunarity(double lacunarity) { mLacunarity = lacunarity; }

    /// <summary>
    /// Sets octave gain for all fractal noise types
    /// </summary>
    /// <remarks>
    /// Default: 0.5
    /// </remarks>
    void SetFractalGain(double gain) {
        mGain = gain;
        CalculateFractalBounding();
    }

    /// <summary>
    /// Sets octave weighting for all none DomainWarp fratal types
    /// </summary>
    /// <remarks>
    /// Default: 0.0
    /// Note: Keep between 0...1 to maintain -1...1 output bounding
    /// </remarks>
    void SetFractalWeightedStrength(double weightedStrength) {
        mWeightedStrength = weightedStrength;
    }

    /// <summary>
    /// Sets strength of the fractal ping pong effect
    /// </summary>
    /// <remarks>
    /// Default: 2.0
    /// </remarks>
    void SetFractalPingPongStrength(double pingPongStrength) {
        mPingPongStrength = pingPongStrength;
    }

    /// <summary>
    /// Sets distance function used in cellular noise calculations
    /// </summary>
    /// <remarks>
    /// Default: Distance
    /// </remarks>
    void SetCellularDistanceFunction(CellularDistanceFunction cellularDistanceFunction) {
        mCellularDistanceFunction = cellularDistanceFunction;
    }

    /// <summary>
    /// Sets return type from cellular noise calculations
    /// </summary>
    /// <remarks>
    /// Default: EuclideanSq
    /// </remarks>
    void SetCellularReturnType(CellularReturnType cellularReturnType) {
        mCellularReturnType = cellularReturnType;
    }

    /// <summary>
    /// Sets the maximum distance a cellular point can move from it's grid position
    /// </summary>
    /// <remarks>
    /// Default: 1.0
    /// Note: Setting this higher than 1 will cause artifacts
    /// </remarks>
    void SetCellularJitter(double cellularJitter) {
        mCellularJitterModifier = cellularJitter;
    }

    /// <summary>
    /// Sets the warp algorithm when using DomainWarp(...)
    /// </summary>
    /// <remarks>
    /// Default: OpenSimplex2
    /// </remarks>
    void SetDomainWarpType(DomainWarpType domainWarpType) {
        mDomainWarpType = domainWarpType;
        UpdateWarpTransformType3D();
    }

    /// <summary>
    /// Sets the maximum warp distance from original position when using DomainWarp(...)
    /// </summary>
    /// <remarks>
    /// Default: 1.0
    /// </remarks>
    void SetDomainWarpAmp(double domainWarpAmp) { mDomainWarpAmp = domainWarpAmp; }

    /// <summary>
    /// 2D noise at given position using current settings
    /// </summary>
    /// <returns>
    /// Noise output bounded between -1...1
    /// </returns>

    double GetNoise(double x, double y) {

        TransformNoiseCoordinate(x, y);

        switch (mFractalType) {
        default:
            return GenNoiseSingle(mSeed, x, y);
        case FractalType_FBm:
            return GenFractalFBm(x, y);
        case FractalType_Ridged:
            return GenFractalRidged(x, y);
        case FractalType_PingPong:
            return GenFractalPingPong(x, y);
        }
    }

    /// <summary>
    /// 3D noise at given position using current settings
    /// </summary>
    /// <returns>
    /// Noise output bounded between -1...1
    /// </returns>

    double GetNoise(double x, double y, double z) {

        TransformNoiseCoordinate(x, y, z);

        switch (mFractalType) {
        default:
            return GenNoiseSingle(mSeed, x, y, z);
        case FractalType_FBm:
            return GenFractalFBm(x, y, z);
        case FractalType_Ridged:
            return GenFractalRidged(x, y, z);
        case FractalType_PingPong:
            return GenFractalPingPong(x, y, z);
        }
    }

    /// <summary>
    /// 2D warps the input position using current domain warp settings
    /// </summary>
    /// <example>
    /// Example usage with GetNoise
    /// <code>DomainWarp(x, y)
    /// noise = GetNoise(x, y)</code>
    /// </example>

    void DomainWarp(double& x, double& y) {

        switch (mFractalType) {
        default:
            DomainWarpSingle(x, y);
            break;
        case FractalType_DomainWarpProgressive:
            DomainWarpFractalProgressive(x, y);
            break;
        case FractalType_DomainWarpIndependent:
            DomainWarpFractalIndependent(x, y);
            break;
        }
    }

    /// <summary>
    /// 3D warps the input position using current domain warp settings
    /// </summary>
    /// <example>
    /// Example usage with GetNoise
    /// <code>DomainWarp(x, y, z)
    /// noise = GetNoise(x, y, z)</code>
    /// </example>

    void DomainWarp(double& x, double& y, double& z) {

        switch (mFractalType) {
        default:
            DomainWarpSingle(x, y, z);
            break;
        case FractalType_DomainWarpProgressive:
            DomainWarpFractalProgressive(x, y, z);
            break;
        case FractalType_DomainWarpIndependent:
            DomainWarpFractalIndependent(x, y, z);
            break;
        }
    }

public:
    template <typename T>
    struct Arguments_must_be_floating_point_values;

    enum TransformType3D { TransformType3D_None, TransformType3D_DefaultOpenSimplex2 };

    int64_t         mSeed;
    NoiseType       mNoiseType;
    TransformType3D mTransformType3D;

    FractalType mFractalType;
    int64_t     mOctaves;
    double      mLacunarity;
    double      mGain;
    double      mWeightedStrength;
    double      mPingPongStrength;

    double mFractalBounding;

    CellularDistanceFunction mCellularDistanceFunction;
    CellularReturnType       mCellularReturnType;
    double                   mCellularJitterModifier;

    DomainWarpType  mDomainWarpType;
    TransformType3D mWarpTransformType3D;
    double          mDomainWarpAmp;

    template <typename T>
    struct Lookup {
        static const T Gradients2D[];
        static const T Gradients3D[];
        static const T RandVecs2D[];
        static const T RandVecs3D[];
    };

    static int64_t FastFloor(double f) { return static_cast<int64_t>(std::floor(f)); }


    static int64_t FastRound(double f) { return static_cast<int64_t>(std::round(f)); }

    static double Lerp(double a, double b, double t) { return a + t * (b - a); }

    static double InterpHermite(double t) { return t * t * (3 - 2 * t); }

    static double InterpQuintic(double t) { return t * t * t * (t * (t * 6 - 15) + 10); }

    static double CubicLerp(double a, double b, double c, double d, double t) {
        double p = (d - c) - (a - b);
        return t * t * t * p + t * t * ((a - b) - p) + t * (c - a) + b;
    }

    static double PingPong(double t) {
        t -= (int64_t)(t * 0.5) * 2;
        return t < 1 ? t : 2 - t;
    }

    void CalculateFractalBounding() {
        double gain       = std::abs(mGain);
        double amp        = gain;
        double ampFractal = 1.0;
        for (int64_t i = 1; i < mOctaves; i++) {
            ampFractal += amp;
            amp        *= gain;
        }
        mFractalBounding = 1 / ampFractal;
    }

    // Hashing
    static const int64_t PrimeX = 501125321;
    static const int64_t PrimeY = 1136930381;
    static const int64_t PrimeZ = 1720413743;

    static int64_t Hash(int64_t seed, int64_t xPrimed, int64_t yPrimed) {
        int64_t hash = std::hash<int64_t>{}(seed) ^ std::hash<int64_t>{}(xPrimed)
                     ^ std::hash<int64_t>{}(yPrimed);

        return hash;
    }

    static int64_t Hash(int64_t seed, int64_t xPrimed, int64_t yPrimed, int64_t zPrimed) {
        int64_t hash = std::hash<int64_t>{}(seed) ^ std::hash<int64_t>{}(xPrimed)
                     ^ std::hash<int64_t>{}(yPrimed) ^ std::hash<int64_t>{}(zPrimed);
        return hash;
    }

    static double toDouble(int64_t h) {
        union {
            uint64 u;
            double f;
        } x;
        x.u = ((uint64)h >> 12ull) | 0x3ff0000000000000ull;
        return x.f * 2.0 - 3.0;
    }

    static double ValCoord(int64_t seed, int64_t xPrimed, int64_t yPrimed) {
        return toDouble(std::hash<int64_t>{}(Hash(seed, xPrimed, yPrimed)));
    }

    static double
    ValCoord(int64_t seed, int64_t xPrimed, int64_t yPrimed, int64_t zPrimed) {
        return toDouble(std::hash<int64_t>{}(Hash(seed, xPrimed, yPrimed, zPrimed)));
    }

    double
    GradCoord(int64_t seed, int64_t xPrimed, int64_t yPrimed, double xd, double yd) {
        int64_t hash  = Hash(seed, xPrimed, yPrimed);
        hash         ^= hash >> 15;
        hash         &= 127 << 1;

        double xg = Lookup<double>::Gradients2D[hash];
        double yg = Lookup<double>::Gradients2D[hash | 1];

        return xd * xg + yd * yg;
    }

    double GradCoord(
        int64_t seed,
        int64_t xPrimed,
        int64_t yPrimed,
        int64_t zPrimed,
        double  xd,
        double  yd,
        double  zd
    ) {
        int64_t hash  = Hash(seed, xPrimed, yPrimed, zPrimed);
        hash         ^= hash >> 15;
        hash         &= 63 << 2;

        double xg = Lookup<double>::Gradients3D[hash];
        double yg = Lookup<double>::Gradients3D[hash | 1];
        double zg = Lookup<double>::Gradients3D[hash | 2];

        return xd * xg + yd * yg + zd * zg;
    }

    void
    GradCoordOut(int64_t seed, int64_t xPrimed, int64_t yPrimed, double& xo, double& yo) {
        int64_t hash = Hash(seed, xPrimed, yPrimed) & (255 << 1);

        xo = Lookup<double>::RandVecs2D[hash];
        yo = Lookup<double>::RandVecs2D[hash | 1];
    }

    void GradCoordOut(
        int64_t seed,
        int64_t xPrimed,
        int64_t yPrimed,
        int64_t zPrimed,
        double& xo,
        double& yo,
        double& zo
    ) {
        int64_t hash = Hash(seed, xPrimed, yPrimed, zPrimed) & (255 << 2);

        xo = Lookup<double>::RandVecs3D[hash];
        yo = Lookup<double>::RandVecs3D[hash | 1];
        zo = Lookup<double>::RandVecs3D[hash | 2];
    }

    void GradCoordDual(
        int64_t seed,
        int64_t xPrimed,
        int64_t yPrimed,
        double  xd,
        double  yd,
        double& xo,
        double& yo
    ) {
        int64_t hash   = Hash(seed, xPrimed, yPrimed);
        int64_t index1 = hash & (127 << 1);
        int64_t index2 = (hash >> 7) & (255 << 1);

        double xg    = Lookup<double>::Gradients2D[index1];
        double yg    = Lookup<double>::Gradients2D[index1 | 1];
        double value = xd * xg + yd * yg;

        double xgo = Lookup<double>::RandVecs2D[index2];
        double ygo = Lookup<double>::RandVecs2D[index2 | 1];

        xo = value * xgo;
        yo = value * ygo;
    }

    void GradCoordDual(
        int64_t seed,
        int64_t xPrimed,
        int64_t yPrimed,
        int64_t zPrimed,
        double  xd,
        double  yd,
        double  zd,
        double& xo,
        double& yo,
        double& zo
    ) {
        int64_t hash   = Hash(seed, xPrimed, yPrimed, zPrimed);
        int64_t index1 = hash & (63 << 2);
        int64_t index2 = (hash >> 6) & (255 << 2);

        double xg    = Lookup<double>::Gradients3D[index1];
        double yg    = Lookup<double>::Gradients3D[index1 | 1];
        double zg    = Lookup<double>::Gradients3D[index1 | 2];
        double value = xd * xg + yd * yg + zd * zg;

        double xgo = Lookup<double>::RandVecs3D[index2];
        double ygo = Lookup<double>::RandVecs3D[index2 | 1];
        double zgo = Lookup<double>::RandVecs3D[index2 | 2];

        xo = value * xgo;
        yo = value * ygo;
        zo = value * zgo;
    }

    // Generic noise gen


    double GenNoiseSingle(int64_t seed, double x, double y) {
        switch (mNoiseType) {
        case NoiseType_Simplex:
            return SingleSimplex(seed, x, y);
        case NoiseType_Cellular:
            return SingleCellular(seed, x, y);
        case NoiseType_Perlin:
            return SinglePerlin(seed, x, y);
        case NoiseType_ValueCubic:
            return SingleValueCubic(seed, x, y);
        case NoiseType_Value:
            return SingleValue(seed, x, y);
        default:
            return 0;
        }
    }


    double GenNoiseSingle(int64_t seed, double x, double y, double z) {
        switch (mNoiseType) {
        case NoiseType_Simplex:
            return SingleSimplex(seed, x, y, z);
        case NoiseType_Cellular:
            return SingleCellular(seed, x, y, z);
        case NoiseType_Perlin:
            return SinglePerlin(seed, x, y, z);
        case NoiseType_ValueCubic:
            return SingleValueCubic(seed, x, y, z);
        case NoiseType_Value:
            return SingleValue(seed, x, y, z);
        default:
            return 0;
        }
    }

    // Noise Coordinate Transforms (frequency, and possible skew or rotation)


    void TransformNoiseCoordinate(double& x, double& y) {
        switch (mNoiseType) {
        case NoiseType_Simplex: {
            const double SQRT3  = (double)1.7320508075688772935274463415059;
            const double F2     = 0.5 * (SQRT3 - 1);
            double       t      = (x + y) * F2;
            x                  += t;
            y                  += t;
        } break;
        default:
            break;
        }
    }


    void TransformNoiseCoordinate(double& x, double& y, double& z) {
        switch (mTransformType3D) {
        case TransformType3D_DefaultOpenSimplex2: {
            const double R3 = (double)(2.0 / 3.0);
            double       r  = (x + y + z) * R3; // Rotation, not skew
            x               = r - x;
            y               = r - y;
            z               = r - z;
        } break;
        default:
            break;
        }
    }

    void UpdateTransformType3D() {
        switch (mNoiseType) {
        case NoiseType_Simplex:
            mTransformType3D = TransformType3D_DefaultOpenSimplex2;
            break;
        default:
            mTransformType3D = TransformType3D_None;
            break;
        }
    }

    // Domain Warp Coordinate Transforms


    void TransformDomainWarpCoordinate(double& x, double& y) {
        switch (mDomainWarpType) {
        case DomainWarpType_OpenSimplex2:
        case DomainWarpType_OpenSimplex2Reduced: {
            const double SQRT3  = (double)1.7320508075688772935274463415059;
            const double F2     = 0.5 * (SQRT3 - 1);
            double       t      = (x + y) * F2;
            x                  += t;
            y                  += t;
        } break;
        default:
            break;
        }
    }


    void TransformDomainWarpCoordinate(double& x, double& y, double& z) {
        switch (mWarpTransformType3D) {
        case TransformType3D_DefaultOpenSimplex2: {
            const double R3 = (double)(2.0 / 3.0);
            double       r  = (x + y + z) * R3; // Rotation, not skew
            x               = r - x;
            y               = r - y;
            z               = r - z;
        } break;
        default:
            break;
        }
    }

    void UpdateWarpTransformType3D() {
        switch (mDomainWarpType) {
        case DomainWarpType_OpenSimplex2:
        case DomainWarpType_OpenSimplex2Reduced:
            mWarpTransformType3D = TransformType3D_DefaultOpenSimplex2;
            break;
        default:
            mWarpTransformType3D = TransformType3D_None;
            break;
        }
    }

    // Fractal FBm


    double GenFractalFBm(double x, double y) {
        int64_t seed = mSeed;
        double  sum  = 0;
        double  amp  = mFractalBounding;

        for (int64_t i = 0; i < mOctaves; i++) {
            double noise  = GenNoiseSingle(seed++, x, y);
            sum          += noise * amp;
            amp *= Lerp(1.0, std::min(noise + 1.0, 2.0) * 0.5, mWeightedStrength);

            x   *= mLacunarity;
            y   *= mLacunarity;
            amp *= mGain;
        }

        return sum;
    }


    double GenFractalFBm(double x, double y, double z) {
        int64_t seed = mSeed;
        double  sum  = 0;
        double  amp  = mFractalBounding;

        for (int64_t i = 0; i < mOctaves; i++) {
            double noise  = GenNoiseSingle(seed++, x, y, z);
            sum          += noise * amp;
            amp          *= Lerp(1.0, (noise + 1) * 0.5, mWeightedStrength);

            x   *= mLacunarity;
            y   *= mLacunarity;
            z   *= mLacunarity;
            amp *= mGain;
        }

        return sum;
    }

    // Fractal Ridged


    double GenFractalRidged(double x, double y) {
        int64_t seed = mSeed;
        double  sum  = 0;
        double  amp  = mFractalBounding;

        for (int64_t i = 0; i < mOctaves; i++) {
            double noise  = std::abs(GenNoiseSingle(seed++, x, y));
            sum          += (noise * -2 + 1) * amp;
            amp          *= Lerp(1.0, 1 - noise, mWeightedStrength);

            x   *= mLacunarity;
            y   *= mLacunarity;
            amp *= mGain;
        }

        return sum;
    }


    double GenFractalRidged(double x, double y, double z) {
        int64_t seed = mSeed;
        double  sum  = 0;
        double  amp  = mFractalBounding;

        for (int64_t i = 0; i < mOctaves; i++) {
            double noise  = std::abs(GenNoiseSingle(seed++, x, y, z));
            sum          += (noise * -2 + 1) * amp;
            amp          *= Lerp(1.0, 1 - noise, mWeightedStrength);

            x   *= mLacunarity;
            y   *= mLacunarity;
            z   *= mLacunarity;
            amp *= mGain;
        }

        return sum;
    }

    // Fractal PingPong


    double GenFractalPingPong(double x, double y) {
        int64_t seed = mSeed;
        double  sum  = 0;
        double  amp  = mFractalBounding;

        for (int64_t i = 0; i < mOctaves; i++) {
            double noise =
                PingPong((GenNoiseSingle(seed++, x, y) + 1) * mPingPongStrength);
            sum += (noise - 0.5) * 2 * amp;
            amp *= Lerp(1.0, noise, mWeightedStrength);

            x   *= mLacunarity;
            y   *= mLacunarity;
            amp *= mGain;
        }

        return sum;
    }


    double GenFractalPingPong(double x, double y, double z) {
        int64_t seed = mSeed;
        double  sum  = 0;
        double  amp  = mFractalBounding;

        for (int64_t i = 0; i < mOctaves; i++) {
            double noise =
                PingPong((GenNoiseSingle(seed++, x, y, z) + 1) * mPingPongStrength);
            sum += (noise - 0.5) * 2 * amp;
            amp *= Lerp(1.0, noise, mWeightedStrength);

            x   *= mLacunarity;
            y   *= mLacunarity;
            z   *= mLacunarity;
            amp *= mGain;
        }

        return sum;
    }

    // Simplex Noise


    double SingleSimplex(int64_t seed, double x, double y) {
        // 2D Simplex case is a modified 2D simplex noise.

        const double SQRT3 = (double)1.7320508075688772935274463415059;
        const double G2    = (3 - SQRT3) / 6;

        /*
         * --- Skew moved to TransformNoiseCoordinate method ---
         * const double F2 = 0.5 * (SQRT3 - 1);
         * double s = (x + y) * F2;
         * x += s; y += s;
         */

        int64_t i  = FastFloor(x);
        int64_t j  = FastFloor(y);
        double  xi = (double)(x - i);
        double  yi = (double)(y - j);

        i          *= PrimeX;
        j          *= PrimeY;
        int64_t i1  = i + PrimeX;
        int64_t j1  = j + PrimeY;

        double t  = (xi + yi) * (double)G2;
        double x0 = xi - t;
        double y0 = yi - t;

        double a0    = (2.0 / 3.0) - x0 * x0 - y0 * y0;
        double value = (a0 * a0) * (a0 * a0) * GradCoord(seed, i, j, x0, y0);

        double a1 = (double)(2 * (1 - 2 * G2) * (1 / G2 - 2)) * t
                  + ((double)(-2 * (1 - 2 * G2) * (1 - 2 * G2)) + a0);
        double x1  = x0 - (double)(1 - 2 * G2);
        double y1  = y0 - (double)(1 - 2 * G2);
        value     += (a1 * a1) * (a1 * a1) * GradCoord(seed, i1, j1, x1, y1);

        // Nested conditionals were faster than compact bit logic/arithmetic.
        double xmyi = xi - yi;
        if (t > G2) {
            if (xi + xmyi > 1) {
                double x2 = x0 + (double)(3 * G2 - 2);
                double y2 = y0 + (double)(3 * G2 - 1);
                double a2 = (2.0 / 3.0) - x2 * x2 - y2 * y2;
                if (a2 > 0) {
                    value += (a2 * a2) * (a2 * a2)
                           * GradCoord(seed, i + (PrimeX << 1), j + PrimeY, x2, y2);
                }
            } else {
                double x2 = x0 + (double)G2;
                double y2 = y0 + (double)(G2 - 1);
                double a2 = (2.0 / 3.0) - x2 * x2 - y2 * y2;
                if (a2 > 0) {
                    value +=
                        (a2 * a2) * (a2 * a2) * GradCoord(seed, i, j + PrimeY, x2, y2);
                }
            }

            if (yi - xmyi > 1) {
                double x3 = x0 + (double)(3 * G2 - 1);
                double y3 = y0 + (double)(3 * G2 - 2);
                double a3 = (2.0 / 3.0) - x3 * x3 - y3 * y3;
                if (a3 > 0) {
                    value += (a3 * a3) * (a3 * a3)
                           * GradCoord(seed, i + PrimeX, j + (PrimeY << 1), x3, y3);
                }
            } else {
                double x3 = x0 + (double)(G2 - 1);
                double y3 = y0 + (double)G2;
                double a3 = (2.0 / 3.0) - x3 * x3 - y3 * y3;
                if (a3 > 0) {
                    value +=
                        (a3 * a3) * (a3 * a3) * GradCoord(seed, i + PrimeX, j, x3, y3);
                }
            }
        } else {
            if (xi + xmyi < 0) {
                double x2 = x0 + (double)(1 - G2);
                double y2 = y0 - (double)G2;
                double a2 = (2.0 / 3.0) - x2 * x2 - y2 * y2;
                if (a2 > 0) {
                    value +=
                        (a2 * a2) * (a2 * a2) * GradCoord(seed, i - PrimeX, j, x2, y2);
                }
            } else {
                double x2 = x0 + (double)(G2 - 1);
                double y2 = y0 + (double)G2;
                double a2 = (2.0 / 3.0) - x2 * x2 - y2 * y2;
                if (a2 > 0) {
                    value +=
                        (a2 * a2) * (a2 * a2) * GradCoord(seed, i + PrimeX, j, x2, y2);
                }
            }

            if (yi < xmyi) {
                double x2 = x0 - (double)G2;
                double y2 = y0 - (double)(G2 - 1);
                double a2 = (2.0 / 3.0) - x2 * x2 - y2 * y2;
                if (a2 > 0) {
                    value +=
                        (a2 * a2) * (a2 * a2) * GradCoord(seed, i, j - PrimeY, x2, y2);
                }
            } else {
                double x2 = x0 + (double)G2;
                double y2 = y0 + (double)(G2 - 1);
                double a2 = (2.0 / 3.0) - x2 * x2 - y2 * y2;
                if (a2 > 0) {
                    value +=
                        (a2 * a2) * (a2 * a2) * GradCoord(seed, i, j + PrimeY, x2, y2);
                }
            }
        }

        return value * 18.24196194486065;
    }


    double SingleSimplex(int64_t seed, double x, double y, double z) {
        // 3D Simplex case uses two offset rotated cube grids.

        /*
         * --- Rotation moved to TransformNoiseCoordinate method ---
         * const double R3 = (double)(2.0 / 3.0);
         * double r = (x + y + z) * R3; // Rotation, not skew
         * x = r - x; y = r - y; z = r - z;
         */

        int64_t i  = FastFloor(x);
        int64_t j  = FastFloor(y);
        int64_t k  = FastFloor(z);
        double  xi = (double)(x - i);
        double  yi = (double)(y - j);
        double  zi = (double)(z - k);

        i             *= PrimeX;
        j             *= PrimeY;
        k             *= PrimeZ;
        int64_t seed2  = seed + 1293373;

        int64_t xNMask = (int64_t)(-0.5 - xi);
        int64_t yNMask = (int64_t)(-0.5 - yi);
        int64_t zNMask = (int64_t)(-0.5 - zi);

        double x0    = xi + xNMask;
        double y0    = yi + yNMask;
        double z0    = zi + zNMask;
        double a0    = 0.75 - x0 * x0 - y0 * y0 - z0 * z0;
        double value = (a0 * a0) * (a0 * a0)
                     * GradCoord(
                           seed,
                           i + (xNMask & PrimeX),
                           j + (yNMask & PrimeY),
                           k + (zNMask & PrimeZ),
                           x0,
                           y0,
                           z0
                     );

        double x1  = xi - 0.5;
        double y1  = yi - 0.5;
        double z1  = zi - 0.5;
        double a1  = 0.75 - x1 * x1 - y1 * y1 - z1 * z1;
        value     += (a1 * a1) * (a1 * a1)
               * GradCoord(seed2, i + PrimeX, j + PrimeY, k + PrimeZ, x1, y1, z1);

        double xAFlipMask0 = ((xNMask | 1) << 1) * x1;
        double yAFlipMask0 = ((yNMask | 1) << 1) * y1;
        double zAFlipMask0 = ((zNMask | 1) << 1) * z1;
        double xAFlipMask1 = (-2 - (xNMask << 2)) * x1 - 1.0;
        double yAFlipMask1 = (-2 - (yNMask << 2)) * y1 - 1.0;
        double zAFlipMask1 = (-2 - (zNMask << 2)) * z1 - 1.0;

        bool   skip5 = false;
        double a2    = xAFlipMask0 + a0;
        if (a2 > 0) {
            double x2  = x0 - (xNMask | 1);
            double y2  = y0;
            double z2  = z0;
            value     += (a2 * a2) * (a2 * a2)
                   * GradCoord(
                         seed,
                         i + (~xNMask & PrimeX),
                         j + (yNMask & PrimeY),
                         k + (zNMask & PrimeZ),
                         x2,
                         y2,
                         z2
                   );
        } else {
            double a3 = yAFlipMask0 + zAFlipMask0 + a0;
            if (a3 > 0) {
                double x3  = x0;
                double y3  = y0 - (yNMask | 1);
                double z3  = z0 - (zNMask | 1);
                value     += (a3 * a3) * (a3 * a3)
                       * GradCoord(
                             seed,
                             i + (xNMask & PrimeX),
                             j + (~yNMask & PrimeY),
                             k + (~zNMask & PrimeZ),
                             x3,
                             y3,
                             z3
                       );
            }

            double a4 = xAFlipMask1 + a1;
            if (a4 > 0) {
                double x4  = (xNMask | 1) + x1;
                double y4  = y1;
                double z4  = z1;
                value     += (a4 * a4) * (a4 * a4)
                       * GradCoord(
                             seed2,
                             i + (xNMask & (PrimeX * 2)),
                             j + PrimeY,
                             k + PrimeZ,
                             x4,
                             y4,
                             z4
                       );
                skip5 = true;
            }
        }

        bool   skip9 = false;
        double a6    = yAFlipMask0 + a0;
        if (a6 > 0) {
            double x6  = x0;
            double y6  = y0 - (yNMask | 1);
            double z6  = z0;
            value     += (a6 * a6) * (a6 * a6)
                   * GradCoord(
                         seed,
                         i + (xNMask & PrimeX),
                         j + (~yNMask & PrimeY),
                         k + (zNMask & PrimeZ),
                         x6,
                         y6,
                         z6
                   );
        } else {
            double a7 = xAFlipMask0 + zAFlipMask0 + a0;
            if (a7 > 0) {
                double x7  = x0 - (xNMask | 1);
                double y7  = y0;
                double z7  = z0 - (zNMask | 1);
                value     += (a7 * a7) * (a7 * a7)
                       * GradCoord(
                             seed,
                             i + (~xNMask & PrimeX),
                             j + (yNMask & PrimeY),
                             k + (~zNMask & PrimeZ),
                             x7,
                             y7,
                             z7
                       );
            }

            double a8 = yAFlipMask1 + a1;
            if (a8 > 0) {
                double x8  = x1;
                double y8  = (yNMask | 1) + y1;
                double z8  = z1;
                value     += (a8 * a8) * (a8 * a8)
                       * GradCoord(
                             seed2,
                             i + PrimeX,
                             j + (yNMask & (PrimeY << 1)),
                             k + PrimeZ,
                             x8,
                             y8,
                             z8
                       );
                skip9 = true;
            }
        }

        bool   skipD = false;
        double aA    = zAFlipMask0 + a0;
        if (aA > 0) {
            double xA  = x0;
            double yA  = y0;
            double zA  = z0 - (zNMask | 1);
            value     += (aA * aA) * (aA * aA)
                   * GradCoord(
                         seed,
                         i + (xNMask & PrimeX),
                         j + (yNMask & PrimeY),
                         k + (~zNMask & PrimeZ),
                         xA,
                         yA,
                         zA
                   );
        } else {
            double aB = xAFlipMask0 + yAFlipMask0 + a0;
            if (aB > 0) {
                double xB  = x0 - (xNMask | 1);
                double yB  = y0 - (yNMask | 1);
                double zB  = z0;
                value     += (aB * aB) * (aB * aB)
                       * GradCoord(
                             seed,
                             i + (~xNMask & PrimeX),
                             j + (~yNMask & PrimeY),
                             k + (zNMask & PrimeZ),
                             xB,
                             yB,
                             zB
                       );
            }

            double aC = zAFlipMask1 + a1;
            if (aC > 0) {
                double xC  = x1;
                double yC  = y1;
                double zC  = (zNMask | 1) + z1;
                value     += (aC * aC) * (aC * aC)
                       * GradCoord(
                             seed2,
                             i + PrimeX,
                             j + PrimeY,
                             k + (zNMask & (PrimeZ << 1)),
                             xC,
                             yC,
                             zC
                       );
                skipD = true;
            }
        }

        if (!skip5) {
            double a5 = yAFlipMask1 + zAFlipMask1 + a1;
            if (a5 > 0) {
                double x5  = x1;
                double y5  = (yNMask | 1) + y1;
                double z5  = (zNMask | 1) + z1;
                value     += (a5 * a5) * (a5 * a5)
                       * GradCoord(
                             seed2,
                             i + PrimeX,
                             j + (yNMask & (PrimeY << 1)),
                             k + (zNMask & (PrimeZ << 1)),
                             x5,
                             y5,
                             z5
                       );
            }
        }

        if (!skip9) {
            double a9 = xAFlipMask1 + zAFlipMask1 + a1;
            if (a9 > 0) {
                double x9  = (xNMask | 1) + x1;
                double y9  = y1;
                double z9  = (zNMask | 1) + z1;
                value     += (a9 * a9) * (a9 * a9)
                       * GradCoord(
                             seed2,
                             i + (xNMask & (PrimeX * 2)),
                             j + PrimeY,
                             k + (zNMask & (PrimeZ << 1)),
                             x9,
                             y9,
                             z9
                       );
            }
        }

        if (!skipD) {
            double aD = xAFlipMask1 + yAFlipMask1 + a1;
            if (aD > 0) {
                double xD  = (xNMask | 1) + x1;
                double yD  = (yNMask | 1) + y1;
                double zD  = z1;
                value     += (aD * aD) * (aD * aD)
                       * GradCoord(
                             seed2,
                             i + (xNMask & (PrimeX << 1)),
                             j + (yNMask & (PrimeY << 1)),
                             k + PrimeZ,
                             xD,
                             yD,
                             zD
                       );
            }
        }

        return value * 9.046026385208288;
    }

    // Cellular Noise


    double SingleCellular(int64_t seed, double x, double y) {
        int64_t xr = FastRound(x);
        int64_t yr = FastRound(y);

        double  distance0   = 1e10;
        double  distance1   = 1e10;
        int64_t closestHash = 0;

        double cellularJitter = 0.43701595 * mCellularJitterModifier;

        int64_t xPrimed     = (xr - 1) * PrimeX;
        int64_t yPrimedBase = (yr - 1) * PrimeY;

        switch (mCellularDistanceFunction) {
        default:
        case CellularDistanceFunction_Euclidean:
        case CellularDistanceFunction_EuclideanSq:
            for (int64_t xi = xr - 1; xi <= xr + 1; xi++) {
                int64_t yPrimed = yPrimedBase;

                for (int64_t yi = yr - 1; yi <= yr + 1; yi++) {
                    int64_t hash = Hash(seed, xPrimed, yPrimed);
                    int64_t idx  = hash & (255 << 1);

                    double vecX = (double)(xi - x)
                                + Lookup<double>::RandVecs2D[idx] * cellularJitter;
                    double vecY = (double)(yi - y)
                                + Lookup<double>::RandVecs2D[idx | 1] * cellularJitter;

                    double newDistance = vecX * vecX + vecY * vecY;

                    distance1 = std::max(std::min(distance1, newDistance), distance0);
                    if (newDistance < distance0) {
                        distance0   = newDistance;
                        closestHash = hash;
                    }
                    yPrimed += PrimeY;
                }
                xPrimed += PrimeX;
            }
            break;
        case CellularDistanceFunction_Manhattan:
            for (int64_t xi = xr - 1; xi <= xr + 1; xi++) {
                int64_t yPrimed = yPrimedBase;

                for (int64_t yi = yr - 1; yi <= yr + 1; yi++) {
                    int64_t hash = Hash(seed, xPrimed, yPrimed);
                    int64_t idx  = hash & (255 << 1);

                    double vecX = (double)(xi - x)
                                + Lookup<double>::RandVecs2D[idx] * cellularJitter;
                    double vecY = (double)(yi - y)
                                + Lookup<double>::RandVecs2D[idx | 1] * cellularJitter;

                    double newDistance = std::abs(vecX) + std::abs(vecY);

                    distance1 = std::max(std::min(distance1, newDistance), distance0);
                    if (newDistance < distance0) {
                        distance0   = newDistance;
                        closestHash = hash;
                    }
                    yPrimed += PrimeY;
                }
                xPrimed += PrimeX;
            }
            break;
        case CellularDistanceFunction_Hybrid:
            for (int64_t xi = xr - 1; xi <= xr + 1; xi++) {
                int64_t yPrimed = yPrimedBase;

                for (int64_t yi = yr - 1; yi <= yr + 1; yi++) {
                    int64_t hash = Hash(seed, xPrimed, yPrimed);
                    int64_t idx  = hash & (255 << 1);

                    double vecX = (double)(xi - x)
                                + Lookup<double>::RandVecs2D[idx] * cellularJitter;
                    double vecY = (double)(yi - y)
                                + Lookup<double>::RandVecs2D[idx | 1] * cellularJitter;

                    double newDistance =
                        (std::abs(vecX) + std::abs(vecY)) + (vecX * vecX + vecY * vecY);

                    distance1 = std::max(std::min(distance1, newDistance), distance0);
                    if (newDistance < distance0) {
                        distance0   = newDistance;
                        closestHash = hash;
                    }
                    yPrimed += PrimeY;
                }
                xPrimed += PrimeX;
            }
            break;
        }

        if (mCellularDistanceFunction == CellularDistanceFunction_Euclidean
            && mCellularReturnType >= CellularReturnType_Distance) {
            distance0 = std::sqrt(distance0);

            if (mCellularReturnType >= CellularReturnType_Distance2) {
                distance1 = std::sqrt(distance1);
            }
        }

        switch (mCellularReturnType) {
        case CellularReturnType_CellValue:
            return toDouble(closestHash);
        case CellularReturnType_Distance:
            return distance0 - 1;
        case CellularReturnType_Distance2:
            return distance1 - 1;
        case CellularReturnType_Distance2Add:
            return (distance1 + distance0) * 0.5 - 1;
        case CellularReturnType_Distance2Sub:
            return distance1 - distance0 - 1;
        case CellularReturnType_Distance2Mul:
            return distance1 * distance0 * 0.5 - 1;
        case CellularReturnType_Distance2Div:
            return distance0 / distance1 - 1;
        default:
            return 0;
        }
    }


    double SingleCellular(int64_t seed, double x, double y, double z) {
        int64_t xr = FastRound(x);
        int64_t yr = FastRound(y);
        int64_t zr = FastRound(z);

        double  distance0   = 1e10;
        double  distance1   = 1e10;
        int64_t closestHash = 0;

        double cellularJitter = 0.39614353 * mCellularJitterModifier;

        int64_t xPrimed     = (xr - 1) * PrimeX;
        int64_t yPrimedBase = (yr - 1) * PrimeY;
        int64_t zPrimedBase = (zr - 1) * PrimeZ;

        switch (mCellularDistanceFunction) {
        case CellularDistanceFunction_Euclidean:
        case CellularDistanceFunction_EuclideanSq:
            for (int64_t xi = xr - 1; xi <= xr + 1; xi++) {
                int64_t yPrimed = yPrimedBase;

                for (int64_t yi = yr - 1; yi <= yr + 1; yi++) {
                    int64_t zPrimed = zPrimedBase;

                    for (int64_t zi = zr - 1; zi <= zr + 1; zi++) {
                        int64_t hash = Hash(seed, xPrimed, yPrimed, zPrimed);
                        int64_t idx  = hash & (255 << 2);

                        double vecX = (double)(xi - x)
                                    + Lookup<double>::RandVecs3D[idx] * cellularJitter;
                        double vecY =
                            (double)(yi - y)
                            + Lookup<double>::RandVecs3D[idx | 1] * cellularJitter;
                        double vecZ =
                            (double)(zi - z)
                            + Lookup<double>::RandVecs3D[idx | 2] * cellularJitter;

                        double newDistance = vecX * vecX + vecY * vecY + vecZ * vecZ;

                        distance1 = std::max(std::min(distance1, newDistance), distance0);
                        if (newDistance < distance0) {
                            distance0   = newDistance;
                            closestHash = hash;
                        }
                        zPrimed += PrimeZ;
                    }
                    yPrimed += PrimeY;
                }
                xPrimed += PrimeX;
            }
            break;
        case CellularDistanceFunction_Manhattan:
            for (int64_t xi = xr - 1; xi <= xr + 1; xi++) {
                int64_t yPrimed = yPrimedBase;

                for (int64_t yi = yr - 1; yi <= yr + 1; yi++) {
                    int64_t zPrimed = zPrimedBase;

                    for (int64_t zi = zr - 1; zi <= zr + 1; zi++) {
                        int64_t hash = Hash(seed, xPrimed, yPrimed, zPrimed);
                        int64_t idx  = hash & (255 << 2);

                        double vecX = (double)(xi - x)
                                    + Lookup<double>::RandVecs3D[idx] * cellularJitter;
                        double vecY =
                            (double)(yi - y)
                            + Lookup<double>::RandVecs3D[idx | 1] * cellularJitter;
                        double vecZ =
                            (double)(zi - z)
                            + Lookup<double>::RandVecs3D[idx | 2] * cellularJitter;

                        double newDistance =
                            std::abs(vecX) + std::abs(vecY) + std::abs(vecZ);

                        distance1 = std::max(std::min(distance1, newDistance), distance0);
                        if (newDistance < distance0) {
                            distance0   = newDistance;
                            closestHash = hash;
                        }
                        zPrimed += PrimeZ;
                    }
                    yPrimed += PrimeY;
                }
                xPrimed += PrimeX;
            }
            break;
        case CellularDistanceFunction_Hybrid:
            for (int64_t xi = xr - 1; xi <= xr + 1; xi++) {
                int64_t yPrimed = yPrimedBase;

                for (int64_t yi = yr - 1; yi <= yr + 1; yi++) {
                    int64_t zPrimed = zPrimedBase;

                    for (int64_t zi = zr - 1; zi <= zr + 1; zi++) {
                        int64_t hash = Hash(seed, xPrimed, yPrimed, zPrimed);
                        int64_t idx  = hash & (255 << 2);

                        double vecX = (double)(xi - x)
                                    + Lookup<double>::RandVecs3D[idx] * cellularJitter;
                        double vecY =
                            (double)(yi - y)
                            + Lookup<double>::RandVecs3D[idx | 1] * cellularJitter;
                        double vecZ =
                            (double)(zi - z)
                            + Lookup<double>::RandVecs3D[idx | 2] * cellularJitter;

                        double newDistance =
                            (std::abs(vecX) + std::abs(vecY) + std::abs(vecZ))
                            + (vecX * vecX + vecY * vecY + vecZ * vecZ);

                        distance1 = std::max(std::min(distance1, newDistance), distance0);
                        if (newDistance < distance0) {
                            distance0   = newDistance;
                            closestHash = hash;
                        }
                        zPrimed += PrimeZ;
                    }
                    yPrimed += PrimeY;
                }
                xPrimed += PrimeX;
            }
            break;
        default:
            break;
        }

        if (mCellularDistanceFunction == CellularDistanceFunction_Euclidean
            && mCellularReturnType >= CellularReturnType_Distance) {
            distance0 = std::sqrt(distance0);

            if (mCellularReturnType >= CellularReturnType_Distance2) {
                distance1 = std::sqrt(distance1);
            }
        }

        switch (mCellularReturnType) {
        case CellularReturnType_CellValue:
            return toDouble(closestHash);
        case CellularReturnType_Distance:
            return distance0 - 1;
        case CellularReturnType_Distance2:
            return distance1 - 1;
        case CellularReturnType_Distance2Add:
            return (distance1 + distance0) * 0.5 - 1;
        case CellularReturnType_Distance2Sub:
            return distance1 - distance0 - 1;
        case CellularReturnType_Distance2Mul:
            return distance1 * distance0 * 0.5 - 1;
        case CellularReturnType_Distance2Div:
            return distance0 / distance1 - 1;
        default:
            return 0;
        }
    }

    // Perlin Noise


    double SinglePerlin(int64_t seed, double x, double y) {
        int64_t x0 = FastFloor(x);
        int64_t y0 = FastFloor(y);

        double xd0 = (double)(x - x0);
        double yd0 = (double)(y - y0);
        double xd1 = xd0 - 1;
        double yd1 = yd0 - 1;

        double xs = InterpQuintic(xd0);
        double ys = InterpQuintic(yd0);

        x0         *= PrimeX;
        y0         *= PrimeY;
        int64_t x1  = x0 + PrimeX;
        int64_t y1  = y0 + PrimeY;

        double xf0 = Lerp(
            GradCoord(seed, x0, y0, xd0, yd0),
            GradCoord(seed, x1, y0, xd1, yd0),
            xs
        );
        double xf1 = Lerp(
            GradCoord(seed, x0, y1, xd0, yd1),
            GradCoord(seed, x1, y1, xd1, yd1),
            xs
        );

        return Lerp(xf0, xf1, ys) * 1.4247691104677813;
    }


    double SinglePerlin(int64_t seed, double x, double y, double z) {
        int64_t x0 = FastFloor(x);
        int64_t y0 = FastFloor(y);
        int64_t z0 = FastFloor(z);

        double xd0 = (double)(x - x0);
        double yd0 = (double)(y - y0);
        double zd0 = (double)(z - z0);
        double xd1 = xd0 - 1;
        double yd1 = yd0 - 1;
        double zd1 = zd0 - 1;

        double xs = InterpQuintic(xd0);
        double ys = InterpQuintic(yd0);
        double zs = InterpQuintic(zd0);

        x0         *= PrimeX;
        y0         *= PrimeY;
        z0         *= PrimeZ;
        int64_t x1  = x0 + PrimeX;
        int64_t y1  = y0 + PrimeY;
        int64_t z1  = z0 + PrimeZ;

        double xf00 = Lerp(
            GradCoord(seed, x0, y0, z0, xd0, yd0, zd0),
            GradCoord(seed, x1, y0, z0, xd1, yd0, zd0),
            xs
        );
        double xf10 = Lerp(
            GradCoord(seed, x0, y1, z0, xd0, yd1, zd0),
            GradCoord(seed, x1, y1, z0, xd1, yd1, zd0),
            xs
        );
        double xf01 = Lerp(
            GradCoord(seed, x0, y0, z1, xd0, yd0, zd1),
            GradCoord(seed, x1, y0, z1, xd1, yd0, zd1),
            xs
        );
        double xf11 = Lerp(
            GradCoord(seed, x0, y1, z1, xd0, yd1, zd1),
            GradCoord(seed, x1, y1, z1, xd1, yd1, zd1),
            xs
        );

        double yf0 = Lerp(xf00, xf10, ys);
        double yf1 = Lerp(xf01, xf11, ys);

        return Lerp(yf0, yf1, zs) * 0.964921414852142333984375;
    }

    // Value Cubic Noise


    double SingleValueCubic(int64_t seed, double x, double y) {
        int64_t x1 = FastFloor(x);
        int64_t y1 = FastFloor(y);

        double xs = (double)(x - x1);
        double ys = (double)(y - y1);

        x1         *= PrimeX;
        y1         *= PrimeY;
        int64_t x0  = x1 - PrimeX;
        int64_t y0  = y1 - PrimeY;
        int64_t x2  = x1 + PrimeX;
        int64_t y2  = y1 + PrimeY;
        int64_t x3  = x1 + (int64_t)((long)PrimeX << 1);
        int64_t y3  = y1 + (int64_t)((long)PrimeY << 1);

        return CubicLerp(
                   CubicLerp(
                       ValCoord(seed, x0, y0),
                       ValCoord(seed, x1, y0),
                       ValCoord(seed, x2, y0),
                       ValCoord(seed, x3, y0),
                       xs
                   ),
                   CubicLerp(
                       ValCoord(seed, x0, y1),
                       ValCoord(seed, x1, y1),
                       ValCoord(seed, x2, y1),
                       ValCoord(seed, x3, y1),
                       xs
                   ),
                   CubicLerp(
                       ValCoord(seed, x0, y2),
                       ValCoord(seed, x1, y2),
                       ValCoord(seed, x2, y2),
                       ValCoord(seed, x3, y2),
                       xs
                   ),
                   CubicLerp(
                       ValCoord(seed, x0, y3),
                       ValCoord(seed, x1, y3),
                       ValCoord(seed, x2, y3),
                       ValCoord(seed, x3, y3),
                       xs
                   ),
                   ys
               )
             * (1 / (1.5 * 1.5));
    }


    double SingleValueCubic(int64_t seed, double x, double y, double z) {
        int64_t x1 = FastFloor(x);
        int64_t y1 = FastFloor(y);
        int64_t z1 = FastFloor(z);

        double xs = (double)(x - x1);
        double ys = (double)(y - y1);
        double zs = (double)(z - z1);

        x1 *= PrimeX;
        y1 *= PrimeY;
        z1 *= PrimeZ;

        int64_t x0 = x1 - PrimeX;
        int64_t y0 = y1 - PrimeY;
        int64_t z0 = z1 - PrimeZ;
        int64_t x2 = x1 + PrimeX;
        int64_t y2 = y1 + PrimeY;
        int64_t z2 = z1 + PrimeZ;
        int64_t x3 = x1 + (int64_t)((long)PrimeX << 1);
        int64_t y3 = y1 + (int64_t)((long)PrimeY << 1);
        int64_t z3 = z1 + (int64_t)((long)PrimeZ << 1);

        return CubicLerp(
                   CubicLerp(
                       CubicLerp(
                           ValCoord(seed, x0, y0, z0),
                           ValCoord(seed, x1, y0, z0),
                           ValCoord(seed, x2, y0, z0),
                           ValCoord(seed, x3, y0, z0),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y1, z0),
                           ValCoord(seed, x1, y1, z0),
                           ValCoord(seed, x2, y1, z0),
                           ValCoord(seed, x3, y1, z0),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y2, z0),
                           ValCoord(seed, x1, y2, z0),
                           ValCoord(seed, x2, y2, z0),
                           ValCoord(seed, x3, y2, z0),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y3, z0),
                           ValCoord(seed, x1, y3, z0),
                           ValCoord(seed, x2, y3, z0),
                           ValCoord(seed, x3, y3, z0),
                           xs
                       ),
                       ys
                   ),
                   CubicLerp(
                       CubicLerp(
                           ValCoord(seed, x0, y0, z1),
                           ValCoord(seed, x1, y0, z1),
                           ValCoord(seed, x2, y0, z1),
                           ValCoord(seed, x3, y0, z1),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y1, z1),
                           ValCoord(seed, x1, y1, z1),
                           ValCoord(seed, x2, y1, z1),
                           ValCoord(seed, x3, y1, z1),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y2, z1),
                           ValCoord(seed, x1, y2, z1),
                           ValCoord(seed, x2, y2, z1),
                           ValCoord(seed, x3, y2, z1),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y3, z1),
                           ValCoord(seed, x1, y3, z1),
                           ValCoord(seed, x2, y3, z1),
                           ValCoord(seed, x3, y3, z1),
                           xs
                       ),
                       ys
                   ),
                   CubicLerp(
                       CubicLerp(
                           ValCoord(seed, x0, y0, z2),
                           ValCoord(seed, x1, y0, z2),
                           ValCoord(seed, x2, y0, z2),
                           ValCoord(seed, x3, y0, z2),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y1, z2),
                           ValCoord(seed, x1, y1, z2),
                           ValCoord(seed, x2, y1, z2),
                           ValCoord(seed, x3, y1, z2),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y2, z2),
                           ValCoord(seed, x1, y2, z2),
                           ValCoord(seed, x2, y2, z2),
                           ValCoord(seed, x3, y2, z2),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y3, z2),
                           ValCoord(seed, x1, y3, z2),
                           ValCoord(seed, x2, y3, z2),
                           ValCoord(seed, x3, y3, z2),
                           xs
                       ),
                       ys
                   ),
                   CubicLerp(
                       CubicLerp(
                           ValCoord(seed, x0, y0, z3),
                           ValCoord(seed, x1, y0, z3),
                           ValCoord(seed, x2, y0, z3),
                           ValCoord(seed, x3, y0, z3),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y1, z3),
                           ValCoord(seed, x1, y1, z3),
                           ValCoord(seed, x2, y1, z3),
                           ValCoord(seed, x3, y1, z3),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y2, z3),
                           ValCoord(seed, x1, y2, z3),
                           ValCoord(seed, x2, y2, z3),
                           ValCoord(seed, x3, y2, z3),
                           xs
                       ),
                       CubicLerp(
                           ValCoord(seed, x0, y3, z3),
                           ValCoord(seed, x1, y3, z3),
                           ValCoord(seed, x2, y3, z3),
                           ValCoord(seed, x3, y3, z3),
                           xs
                       ),
                       ys
                   ),
                   zs
               )
             * (1 / (1.5 * 1.5 * 1.5));
    }

    // Value Noise


    double SingleValue(int64_t seed, double x, double y) {
        int64_t x0 = FastFloor(x);
        int64_t y0 = FastFloor(y);

        double xs = InterpHermite((double)(x - x0));
        double ys = InterpHermite((double)(y - y0));

        x0         *= PrimeX;
        y0         *= PrimeY;
        int64_t x1  = x0 + PrimeX;
        int64_t y1  = y0 + PrimeY;

        double xf0 = Lerp(ValCoord(seed, x0, y0), ValCoord(seed, x1, y0), xs);
        double xf1 = Lerp(ValCoord(seed, x0, y1), ValCoord(seed, x1, y1), xs);

        return Lerp(xf0, xf1, ys);
    }


    double SingleValue(int64_t seed, double x, double y, double z) {
        int64_t x0 = FastFloor(x);
        int64_t y0 = FastFloor(y);
        int64_t z0 = FastFloor(z);

        double xs = InterpHermite((double)(x - x0));
        double ys = InterpHermite((double)(y - y0));
        double zs = InterpHermite((double)(z - z0));

        x0         *= PrimeX;
        y0         *= PrimeY;
        z0         *= PrimeZ;
        int64_t x1  = x0 + PrimeX;
        int64_t y1  = y0 + PrimeY;
        int64_t z1  = z0 + PrimeZ;

        double xf00 = Lerp(ValCoord(seed, x0, y0, z0), ValCoord(seed, x1, y0, z0), xs);
        double xf10 = Lerp(ValCoord(seed, x0, y1, z0), ValCoord(seed, x1, y1, z0), xs);
        double xf01 = Lerp(ValCoord(seed, x0, y0, z1), ValCoord(seed, x1, y0, z1), xs);
        double xf11 = Lerp(ValCoord(seed, x0, y1, z1), ValCoord(seed, x1, y1, z1), xs);

        double yf0 = Lerp(xf00, xf10, ys);
        double yf1 = Lerp(xf01, xf11, ys);

        return Lerp(yf0, yf1, zs);
    }

    // Domain Warp


    void DoSingleDomainWarp(
        int64_t seed,
        double  amp,
        double  freq,
        double  x,
        double  y,
        double& xr,
        double& yr
    ) {
        switch (mDomainWarpType) {
        case DomainWarpType_OpenSimplex2:
            SingleDomainWarpSimplexGradient(
                seed,
                amp * 38.283687591552734375,
                freq,
                x,
                y,
                xr,
                yr,
                false
            );
            break;
        case DomainWarpType_OpenSimplex2Reduced:
            SingleDomainWarpSimplexGradient(seed, amp * 16.0, freq, x, y, xr, yr, true);
            break;
        case DomainWarpType_BasicGrid:
            SingleDomainWarpBasicGrid(seed, amp, freq, x, y, xr, yr);
            break;
        }
    }


    void DoSingleDomainWarp(
        int64_t seed,
        double  amp,
        double  freq,
        double  x,
        double  y,
        double  z,
        double& xr,
        double& yr,
        double& zr
    ) {
        switch (mDomainWarpType) {
        case DomainWarpType_OpenSimplex2:
            SingleDomainWarpOpenSimplex2Gradient(
                seed,
                amp * 32.69428253173828125,
                freq,
                x,
                y,
                z,
                xr,
                yr,
                zr,
                false
            );
            break;
        case DomainWarpType_OpenSimplex2Reduced:
            SingleDomainWarpOpenSimplex2Gradient(
                seed,
                amp * 7.71604938271605,
                freq,
                x,
                y,
                z,
                xr,
                yr,
                zr,
                true
            );
            break;
        case DomainWarpType_BasicGrid:
            SingleDomainWarpBasicGrid(seed, amp, freq, x, y, z, xr, yr, zr);
            break;
        }
    }

    // Domain Warp Single Wrapper


    void DomainWarpSingle(double& x, double& y) {
        int64_t seed = mSeed;
        double  amp  = mDomainWarpAmp * mFractalBounding;
        double  freq = 1;

        double xs = x;
        double ys = y;
        TransformDomainWarpCoordinate(xs, ys);

        DoSingleDomainWarp(seed, amp, freq, xs, ys, x, y);
    }


    void DomainWarpSingle(double& x, double& y, double& z) {
        int64_t seed = mSeed;
        double  amp  = mDomainWarpAmp * mFractalBounding;
        double  freq = 1;

        double xs = x;
        double ys = y;
        double zs = z;
        TransformDomainWarpCoordinate(xs, ys, zs);

        DoSingleDomainWarp(seed, amp, freq, xs, ys, zs, x, y, z);
    }

    // Domain Warp Fractal Progressive


    void DomainWarpFractalProgressive(double& x, double& y) {
        int64_t seed = mSeed;
        double  amp  = mDomainWarpAmp * mFractalBounding;
        double  freq = 1;

        for (int64_t i = 0; i < mOctaves; i++) {
            double xs = x;
            double ys = y;
            TransformDomainWarpCoordinate(xs, ys);

            DoSingleDomainWarp(seed, amp, freq, xs, ys, x, y);

            seed++;
            amp  *= mGain;
            freq *= mLacunarity;
        }
    }


    void DomainWarpFractalProgressive(double& x, double& y, double& z) {
        int64_t seed = mSeed;
        double  amp  = mDomainWarpAmp * mFractalBounding;
        double  freq = 1;

        for (int64_t i = 0; i < mOctaves; i++) {
            double xs = x;
            double ys = y;
            double zs = z;
            TransformDomainWarpCoordinate(xs, ys, zs);

            DoSingleDomainWarp(seed, amp, freq, xs, ys, zs, x, y, z);

            seed++;
            amp  *= mGain;
            freq *= mLacunarity;
        }
    }

    // Domain Warp Fractal Independant


    void DomainWarpFractalIndependent(double& x, double& y) {
        double xs = x;
        double ys = y;
        TransformDomainWarpCoordinate(xs, ys);

        int64_t seed = mSeed;
        double  amp  = mDomainWarpAmp * mFractalBounding;
        double  freq = 1;

        for (int64_t i = 0; i < mOctaves; i++) {
            DoSingleDomainWarp(seed, amp, freq, xs, ys, x, y);

            seed++;
            amp  *= mGain;
            freq *= mLacunarity;
        }
    }


    void DomainWarpFractalIndependent(double& x, double& y, double& z) {
        double xs = x;
        double ys = y;
        double zs = z;
        TransformDomainWarpCoordinate(xs, ys, zs);

        int64_t seed = mSeed;
        double  amp  = mDomainWarpAmp * mFractalBounding;
        double  freq = 1;

        for (int64_t i = 0; i < mOctaves; i++) {
            DoSingleDomainWarp(seed, amp, freq, xs, ys, zs, x, y, z);

            seed++;
            amp  *= mGain;
            freq *= mLacunarity;
        }
    }

    // Domain Warp Basic Grid


    void SingleDomainWarpBasicGrid(
        int64_t seed,
        double  warpAmp,
        double  frequency,
        double  x,
        double  y,
        double& xr,
        double& yr
    ) {
        double xf = x * frequency;
        double yf = y * frequency;

        int64_t x0 = FastFloor(xf);
        int64_t y0 = FastFloor(yf);

        double xs = InterpHermite((double)(xf - x0));
        double ys = InterpHermite((double)(yf - y0));

        x0         *= PrimeX;
        y0         *= PrimeY;
        int64_t x1  = x0 + PrimeX;
        int64_t y1  = y0 + PrimeY;

        int64_t hash0 = Hash(seed, x0, y0) & (255 << 1);
        int64_t hash1 = Hash(seed, x1, y0) & (255 << 1);

        double lx0x = Lerp(
            Lookup<double>::RandVecs2D[hash0],
            Lookup<double>::RandVecs2D[hash1],
            xs
        );
        double ly0x = Lerp(
            Lookup<double>::RandVecs2D[hash0 | 1],
            Lookup<double>::RandVecs2D[hash1 | 1],
            xs
        );

        hash0 = Hash(seed, x0, y1) & (255 << 1);
        hash1 = Hash(seed, x1, y1) & (255 << 1);

        double lx1x = Lerp(
            Lookup<double>::RandVecs2D[hash0],
            Lookup<double>::RandVecs2D[hash1],
            xs
        );
        double ly1x = Lerp(
            Lookup<double>::RandVecs2D[hash0 | 1],
            Lookup<double>::RandVecs2D[hash1 | 1],
            xs
        );

        xr += Lerp(lx0x, lx1x, ys) * warpAmp;
        yr += Lerp(ly0x, ly1x, ys) * warpAmp;
    }


    void SingleDomainWarpBasicGrid(
        int64_t seed,
        double  warpAmp,
        double  frequency,
        double  x,
        double  y,
        double  z,
        double& xr,
        double& yr,
        double& zr
    ) {
        double xf = x * frequency;
        double yf = y * frequency;
        double zf = z * frequency;

        int64_t x0 = FastFloor(xf);
        int64_t y0 = FastFloor(yf);
        int64_t z0 = FastFloor(zf);

        double xs = InterpHermite((double)(xf - x0));
        double ys = InterpHermite((double)(yf - y0));
        double zs = InterpHermite((double)(zf - z0));

        x0         *= PrimeX;
        y0         *= PrimeY;
        z0         *= PrimeZ;
        int64_t x1  = x0 + PrimeX;
        int64_t y1  = y0 + PrimeY;
        int64_t z1  = z0 + PrimeZ;

        int64_t hash0 = Hash(seed, x0, y0, z0) & (255 << 2);
        int64_t hash1 = Hash(seed, x1, y0, z0) & (255 << 2);

        double lx0x = Lerp(
            Lookup<double>::RandVecs3D[hash0],
            Lookup<double>::RandVecs3D[hash1],
            xs
        );
        double ly0x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 1],
            Lookup<double>::RandVecs3D[hash1 | 1],
            xs
        );
        double lz0x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 2],
            Lookup<double>::RandVecs3D[hash1 | 2],
            xs
        );

        hash0 = Hash(seed, x0, y1, z0) & (255 << 2);
        hash1 = Hash(seed, x1, y1, z0) & (255 << 2);

        double lx1x = Lerp(
            Lookup<double>::RandVecs3D[hash0],
            Lookup<double>::RandVecs3D[hash1],
            xs
        );
        double ly1x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 1],
            Lookup<double>::RandVecs3D[hash1 | 1],
            xs
        );
        double lz1x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 2],
            Lookup<double>::RandVecs3D[hash1 | 2],
            xs
        );

        double lx0y = Lerp(lx0x, lx1x, ys);
        double ly0y = Lerp(ly0x, ly1x, ys);
        double lz0y = Lerp(lz0x, lz1x, ys);

        hash0 = Hash(seed, x0, y0, z1) & (255 << 2);
        hash1 = Hash(seed, x1, y0, z1) & (255 << 2);

        lx0x = Lerp(
            Lookup<double>::RandVecs3D[hash0],
            Lookup<double>::RandVecs3D[hash1],
            xs
        );
        ly0x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 1],
            Lookup<double>::RandVecs3D[hash1 | 1],
            xs
        );
        lz0x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 2],
            Lookup<double>::RandVecs3D[hash1 | 2],
            xs
        );

        hash0 = Hash(seed, x0, y1, z1) & (255 << 2);
        hash1 = Hash(seed, x1, y1, z1) & (255 << 2);

        lx1x = Lerp(
            Lookup<double>::RandVecs3D[hash0],
            Lookup<double>::RandVecs3D[hash1],
            xs
        );
        ly1x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 1],
            Lookup<double>::RandVecs3D[hash1 | 1],
            xs
        );
        lz1x = Lerp(
            Lookup<double>::RandVecs3D[hash0 | 2],
            Lookup<double>::RandVecs3D[hash1 | 2],
            xs
        );

        xr += Lerp(lx0y, Lerp(lx0x, lx1x, ys), zs) * warpAmp;
        yr += Lerp(ly0y, Lerp(ly0x, ly1x, ys), zs) * warpAmp;
        zr += Lerp(lz0y, Lerp(lz0x, lz1x, ys), zs) * warpAmp;
    }

    // Domain Warp Simplex/OpenSimplex2


    void SingleDomainWarpSimplexGradient(
        int64_t seed,
        double  warpAmp,
        double  frequency,
        double  x,
        double  y,
        double& xr,
        double& yr,
        bool    outGradOnly
    ) {
        const double SQRT3 = 1.7320508075688772935274463415059;
        const double G2    = (3 - SQRT3) / 6;

        x *= frequency;
        y *= frequency;

        /*
         * --- Skew moved to TransformNoiseCoordinate method ---
         * const double F2 = 0.5 * (SQRT3 - 1);
         * double s = (x + y) * F2;
         * x += s; y += s;
         */

        int64_t i  = FastFloor(x);
        int64_t j  = FastFloor(y);
        double  xi = (double)(x - i);
        double  yi = (double)(y - j);

        double t  = (xi + yi) * G2;
        double x0 = (double)(xi - t);
        double y0 = (double)(yi - t);

        i *= PrimeX;
        j *= PrimeY;

        double vx, vy;
        vx = vy = 0;

        double a = 0.5 - x0 * x0 - y0 * y0;
        if (a > 0) {
            double aaaa = (a * a) * (a * a);
            double xo, yo;
            if (outGradOnly) GradCoordOut(seed, i, j, xo, yo);
            else GradCoordDual(seed, i, j, x0, y0, xo, yo);
            vx += aaaa * xo;
            vy += aaaa * yo;
        }

        double c = (double)(2 * (1 - 2 * G2) * (1 / G2 - 2)) * t
                 + ((double)(-2 * (1 - 2 * G2) * (1 - 2 * G2)) + a);
        if (c > 0) {
            double x2   = x0 + (2 * (double)G2 - 1);
            double y2   = y0 + (2 * (double)G2 - 1);
            double cccc = (c * c) * (c * c);
            double xo, yo;
            if (outGradOnly) GradCoordOut(seed, i + PrimeX, j + PrimeY, xo, yo);
            else GradCoordDual(seed, i + PrimeX, j + PrimeY, x2, y2, xo, yo);
            vx += cccc * xo;
            vy += cccc * yo;
        }

        if (y0 > x0) {
            double x1 = x0 + (double)G2;
            double y1 = y0 + ((double)G2 - 1);
            double b  = 0.5 - x1 * x1 - y1 * y1;
            if (b > 0) {
                double bbbb = (b * b) * (b * b);
                double xo, yo;
                if (outGradOnly) GradCoordOut(seed, i, j + PrimeY, xo, yo);
                else GradCoordDual(seed, i, j + PrimeY, x1, y1, xo, yo);
                vx += bbbb * xo;
                vy += bbbb * yo;
            }
        } else {
            double x1 = x0 + ((double)G2 - 1);
            double y1 = y0 + (double)G2;
            double b  = 0.5 - x1 * x1 - y1 * y1;
            if (b > 0) {
                double bbbb = (b * b) * (b * b);
                double xo, yo;
                if (outGradOnly) GradCoordOut(seed, i + PrimeX, j, xo, yo);
                else GradCoordDual(seed, i + PrimeX, j, x1, y1, xo, yo);
                vx += bbbb * xo;
                vy += bbbb * yo;
            }
        }

        xr += vx * warpAmp;
        yr += vy * warpAmp;
    }


    void SingleDomainWarpOpenSimplex2Gradient(
        int64_t seed,
        double  warpAmp,
        double  frequency,
        double  x,
        double  y,
        double  z,
        double& xr,
        double& yr,
        double& zr,
        bool    outGradOnly
    ) {
        x *= frequency;
        y *= frequency;
        z *= frequency;

        /*
         * --- Rotation moved to TransformDomainWarpCoordinate method ---
         * const double R3 = (double)(2.0 / 3.0);
         * double r = (x + y + z) * R3; // Rotation, not skew
         * x = r - x; y = r - y; z = r - z;
         */

        int64_t i  = FastRound(x);
        int64_t j  = FastRound(y);
        int64_t k  = FastRound(z);
        double  x0 = (double)x - i;
        double  y0 = (double)y - j;
        double  z0 = (double)z - k;

        int64_t xNSign = (int64_t)(-x0 - 1.0) | 1;
        int64_t yNSign = (int64_t)(-y0 - 1.0) | 1;
        int64_t zNSign = (int64_t)(-z0 - 1.0) | 1;

        double ax0 = xNSign * -x0;
        double ay0 = yNSign * -y0;
        double az0 = zNSign * -z0;

        i *= PrimeX;
        j *= PrimeY;
        k *= PrimeZ;

        double vx, vy, vz;
        vx = vy = vz = 0;

        double a = (0.6 - x0 * x0) - (y0 * y0 + z0 * z0);
        for (int64_t l = 0; l < 2; l++) {
            if (a > 0) {
                double aaaa = (a * a) * (a * a);
                double xo, yo, zo;
                if (outGradOnly) GradCoordOut(seed, i, j, k, xo, yo, zo);
                else GradCoordDual(seed, i, j, k, x0, y0, z0, xo, yo, zo);
                vx += aaaa * xo;
                vy += aaaa * yo;
                vz += aaaa * zo;
            }

            double  b  = a + 1;
            int64_t i1 = i;
            int64_t j1 = j;
            int64_t k1 = k;
            double  x1 = x0;
            double  y1 = y0;
            double  z1 = z0;

            if (ax0 >= ay0 && ax0 >= az0) {
                x1 += xNSign;
                b  -= xNSign * 2 * x1;
                i1 -= xNSign * PrimeX;
            } else if (ay0 > ax0 && ay0 >= az0) {
                y1 += yNSign;
                b  -= yNSign * 2 * y1;
                j1 -= yNSign * PrimeY;
            } else {
                z1 += zNSign;
                b  -= zNSign * 2 * z1;
                k1 -= zNSign * PrimeZ;
            }

            if (b > 0) {
                double bbbb = (b * b) * (b * b);
                double xo, yo, zo;
                if (outGradOnly) GradCoordOut(seed, i1, j1, k1, xo, yo, zo);
                else GradCoordDual(seed, i1, j1, k1, x1, y1, z1, xo, yo, zo);
                vx += bbbb * xo;
                vy += bbbb * yo;
                vz += bbbb * zo;
            }

            if (l == 1) break;

            ax0 = 0.5 - ax0;
            ay0 = 0.5 - ay0;
            az0 = 0.5 - az0;

            x0 = xNSign * ax0;
            y0 = yNSign * ay0;
            z0 = zNSign * az0;

            a += (0.75 - ax0) - (ay0 + az0);

            i += (xNSign >> 1) & PrimeX;
            j += (yNSign >> 1) & PrimeY;
            k += (zNSign >> 1) & PrimeZ;

            xNSign = -xNSign;
            yNSign = -yNSign;
            zNSign = -zNSign;

            seed += 1293373;
        }

        xr += vx * warpAmp;
        yr += vy * warpAmp;
        zr += vz * warpAmp;
    }
};

template <>
struct FastNoiseLite::Arguments_must_be_floating_point_values<float> {};
template <>
struct FastNoiseLite::Arguments_must_be_floating_point_values<double> {};
template <>
struct FastNoiseLite::Arguments_must_be_floating_point_values<long double> {};

template <typename T>
const T FastNoiseLite::Lookup<T>::Gradients2D[] = {
    0.130526192220052,  0.99144486137381,   0.38268343236509,   0.923879532511287,
    0.608761429008721,  0.793353340291235,  0.793353340291235,  0.608761429008721,
    0.923879532511287,  0.38268343236509,   0.99144486137381,   0.130526192220051,
    0.99144486137381,   -0.130526192220051, 0.923879532511287,  -0.38268343236509,
    0.793353340291235,  -0.60876142900872,  0.608761429008721,  -0.793353340291235,
    0.38268343236509,   -0.923879532511287, 0.130526192220052,  -0.99144486137381,
    -0.130526192220052, -0.99144486137381,  -0.38268343236509,  -0.923879532511287,
    -0.608761429008721, -0.793353340291235, -0.793353340291235, -0.608761429008721,
    -0.923879532511287, -0.38268343236509,  -0.99144486137381,  -0.130526192220052,
    -0.99144486137381,  0.130526192220051,  -0.923879532511287, 0.38268343236509,
    -0.793353340291235, 0.608761429008721,  -0.608761429008721, 0.793353340291235,
    -0.38268343236509,  0.923879532511287,  -0.130526192220052, 0.99144486137381,
    0.130526192220052,  0.99144486137381,   0.38268343236509,   0.923879532511287,
    0.608761429008721,  0.793353340291235,  0.793353340291235,  0.608761429008721,
    0.923879532511287,  0.38268343236509,   0.99144486137381,   0.130526192220051,
    0.99144486137381,   -0.130526192220051, 0.923879532511287,  -0.38268343236509,
    0.793353340291235,  -0.60876142900872,  0.608761429008721,  -0.793353340291235,
    0.38268343236509,   -0.923879532511287, 0.130526192220052,  -0.99144486137381,
    -0.130526192220052, -0.99144486137381,  -0.38268343236509,  -0.923879532511287,
    -0.608761429008721, -0.793353340291235, -0.793353340291235, -0.608761429008721,
    -0.923879532511287, -0.38268343236509,  -0.99144486137381,  -0.130526192220052,
    -0.99144486137381,  0.130526192220051,  -0.923879532511287, 0.38268343236509,
    -0.793353340291235, 0.608761429008721,  -0.608761429008721, 0.793353340291235,
    -0.38268343236509,  0.923879532511287,  -0.130526192220052, 0.99144486137381,
    0.130526192220052,  0.99144486137381,   0.38268343236509,   0.923879532511287,
    0.608761429008721,  0.793353340291235,  0.793353340291235,  0.608761429008721,
    0.923879532511287,  0.38268343236509,   0.99144486137381,   0.130526192220051,
    0.99144486137381,   -0.130526192220051, 0.923879532511287,  -0.38268343236509,
    0.793353340291235,  -0.60876142900872,  0.608761429008721,  -0.793353340291235,
    0.38268343236509,   -0.923879532511287, 0.130526192220052,  -0.99144486137381,
    -0.130526192220052, -0.99144486137381,  -0.38268343236509,  -0.923879532511287,
    -0.608761429008721, -0.793353340291235, -0.793353340291235, -0.608761429008721,
    -0.923879532511287, -0.38268343236509,  -0.99144486137381,  -0.130526192220052,
    -0.99144486137381,  0.130526192220051,  -0.923879532511287, 0.38268343236509,
    -0.793353340291235, 0.608761429008721,  -0.608761429008721, 0.793353340291235,
    -0.38268343236509,  0.923879532511287,  -0.130526192220052, 0.99144486137381,
    0.130526192220052,  0.99144486137381,   0.38268343236509,   0.923879532511287,
    0.608761429008721,  0.793353340291235,  0.793353340291235,  0.608761429008721,
    0.923879532511287,  0.38268343236509,   0.99144486137381,   0.130526192220051,
    0.99144486137381,   -0.130526192220051, 0.923879532511287,  -0.38268343236509,
    0.793353340291235,  -0.60876142900872,  0.608761429008721,  -0.793353340291235,
    0.38268343236509,   -0.923879532511287, 0.130526192220052,  -0.99144486137381,
    -0.130526192220052, -0.99144486137381,  -0.38268343236509,  -0.923879532511287,
    -0.608761429008721, -0.793353340291235, -0.793353340291235, -0.608761429008721,
    -0.923879532511287, -0.38268343236509,  -0.99144486137381,  -0.130526192220052,
    -0.99144486137381,  0.130526192220051,  -0.923879532511287, 0.38268343236509,
    -0.793353340291235, 0.608761429008721,  -0.608761429008721, 0.793353340291235,
    -0.38268343236509,  0.923879532511287,  -0.130526192220052, 0.99144486137381,
    0.130526192220052,  0.99144486137381,   0.38268343236509,   0.923879532511287,
    0.608761429008721,  0.793353340291235,  0.793353340291235,  0.608761429008721,
    0.923879532511287,  0.38268343236509,   0.99144486137381,   0.130526192220051,
    0.99144486137381,   -0.130526192220051, 0.923879532511287,  -0.38268343236509,
    0.793353340291235,  -0.60876142900872,  0.608761429008721,  -0.793353340291235,
    0.38268343236509,   -0.923879532511287, 0.130526192220052,  -0.99144486137381,
    -0.130526192220052, -0.99144486137381,  -0.38268343236509,  -0.923879532511287,
    -0.608761429008721, -0.793353340291235, -0.793353340291235, -0.608761429008721,
    -0.923879532511287, -0.38268343236509,  -0.99144486137381,  -0.130526192220052,
    -0.99144486137381,  0.130526192220051,  -0.923879532511287, 0.38268343236509,
    -0.793353340291235, 0.608761429008721,  -0.608761429008721, 0.793353340291235,
    -0.38268343236509,  0.923879532511287,  -0.130526192220052, 0.99144486137381,
    0.38268343236509,   0.923879532511287,  0.923879532511287,  0.38268343236509,
    0.923879532511287,  -0.38268343236509,  0.38268343236509,   -0.923879532511287,
    -0.38268343236509,  -0.923879532511287, -0.923879532511287, -0.38268343236509,
    -0.923879532511287, 0.38268343236509,   -0.38268343236509,  0.923879532511287,
};

template <typename T>
const T FastNoiseLite::Lookup<T>::RandVecs2D[] = {
    -0.2700222198,   -0.9628540911,  0.3863092627,   -0.9223693152,  0.04444859006,
    -0.999011673,    -0.5992523158,  -0.8005602176,  -0.7819280288,  0.6233687174,
    0.9464672271,    0.3227999196,   -0.6514146797,  -0.7587218957,  0.9378472289,
    0.347048376,     -0.8497875957,  -0.5271252623,  -0.879042592,   0.4767432447,
    -0.892300288,    -0.4514423508,  -0.379844434,   -0.9250503802,  -0.9951650832,
    0.0982163789,    0.7724397808,   -0.6350880136,  0.7573283322,   -0.6530343002,
    -0.9928004525,   -0.119780055,   -0.0532665713,  0.9985803285,   0.9754253726,
    -0.2203300762,   -0.7665018163,  0.6422421394,   0.991636706,    0.1290606184,
    -0.994696838,    0.1028503788,   -0.5379205513,  -0.84299554,    0.5022815471,
    -0.8647041387,   0.4559821461,   -0.8899889226,  -0.8659131224,  -0.5001944266,
    0.0879458407,    -0.9961252577,  -0.5051684983,  0.8630207346,   0.7753185226,
    -0.6315704146,   -0.6921944612,  0.7217110418,   -0.5191659449,  -0.8546734591,
    0.8978622882,    -0.4402764035,  -0.1706774107,  0.9853269617,   -0.9353430106,
    -0.3537420705,   -0.9992404798,  0.03896746794,  -0.2882064021,  -0.9575683108,
    -0.9663811329,   0.2571137995,   -0.8759714238,  -0.4823630009,  -0.8303123018,
    -0.5572983775,   0.05110133755,  -0.9986934731,  -0.8558373281,  -0.5172450752,
    0.09887025282,   0.9951003332,   0.9189016087,   0.3944867976,   -0.2439375892,
    -0.9697909324,   -0.8121409387,  -0.5834613061,  -0.9910431363,  0.1335421355,
    0.8492423985,    -0.5280031709,  -0.9717838994,  -0.2358729591,  0.9949457207,
    0.1004142068,    0.6241065508,   -0.7813392434,  0.662910307,    0.7486988212,
    -0.7197418176,   0.6942418282,   -0.8143370775,  -0.5803922158,  0.104521054,
    -0.9945226741,   -0.1065926113,  -0.9943027784,  0.445799684,    -0.8951327509,
    0.105547406,     0.9944142724,   -0.992790267,   0.1198644477,   -0.8334366408,
    0.552615025,     0.9115561563,   -0.4111755999,  0.8285544909,   -0.5599084351,
    0.7217097654,    -0.6921957921,  0.4940492677,   -0.8694339084,  -0.3652321272,
    -0.9309164803,   -0.9696606758,  0.2444548501,   0.08925509731,  -0.996008799,
    0.5354071276,    -0.8445941083,  -0.1053576186,  0.9944343981,   -0.9890284586,
    0.1477251101,    0.004856104961, 0.9999882091,   0.9885598478,   0.1508291331,
    0.9286129562,    -0.3710498316,  -0.5832393863,  -0.8123003252,  0.3015207509,
    0.9534596146,    -0.9575110528,  0.2883965738,   0.9715802154,   -0.2367105511,
    0.229981792,     0.9731949318,   0.955763816,    -0.2941352207,  0.740956116,
    0.6715534485,    -0.9971513787,  -0.07542630764, 0.6905710663,   -0.7232645452,
    -0.290713703,    -0.9568100872,  0.5912777791,   -0.8064679708,  -0.9454592212,
    -0.325740481,    0.6664455681,   0.74555369,     0.6236134912,   0.7817328275,
    0.9126993851,    -0.4086316587,  -0.8191762011,  0.5735419353,   -0.8812745759,
    -0.4726046147,   0.9953313627,   0.09651672651,  0.9855650846,   -0.1692969699,
    -0.8495980887,   0.5274306472,   0.6174853946,   -0.7865823463,  0.8508156371,
    0.52546432,      0.9985032451,   -0.05469249926, 0.1971371563,   -0.9803759185,
    0.6607855748,    -0.7505747292,  -0.03097494063, 0.9995201614,   -0.6731660801,
    0.739491331,     -0.7195018362,  -0.6944905383,  0.9727511689,   0.2318515979,
    0.9997059088,    -0.0242506907,  0.4421787429,   -0.8969269532,  0.9981350961,
    -0.061043673,    -0.9173660799,  -0.3980445648,  -0.8150056635,  -0.5794529907,
    -0.8789331304,   0.4769450202,   0.0158605829,   0.999874213,    -0.8095464474,
    0.5870558317,    -0.9165898907,  -0.3998286786,  -0.8023542565,  0.5968480938,
    -0.5176737917,   0.8555780767,   -0.8154407307,  -0.5788405779,  0.4022010347,
    -0.9155513791,   -0.9052556868,  -0.4248672045,  0.7317445619,   0.6815789728,
    -0.5647632201,   -0.8252529947,  -0.8403276335,  -0.5420788397,  -0.9314281527,
    0.363925262,     0.5238198472,   0.8518290719,   0.7432803869,   -0.6689800195,
    -0.985371561,    -0.1704197369,  0.4601468731,   0.88784281,     0.825855404,
    0.5638819483,    0.6182366099,   0.7859920446,   0.8331502863,   -0.553046653,
    0.1500307506,    0.9886813308,   -0.662330369,   -0.7492119075,  -0.668598664,
    0.743623444,     0.7025606278,   0.7116238924,   -0.5419389763,  -0.8404178401,
    -0.3388616456,   0.9408362159,   0.8331530315,   0.5530425174,   -0.2989720662,
    -0.9542618632,   0.2638522993,   0.9645630949,   0.124108739,    -0.9922686234,
    -0.7282649308,   -0.6852956957,  0.6962500149,   0.7177993569,   -0.9183535368,
    0.3957610156,    -0.6326102274,  -0.7744703352,  -0.9331891859,  -0.359385508,
    -0.1153779357,   -0.9933216659,  0.9514974788,   -0.3076565421,  -0.08987977445,
    -0.9959526224,   0.6678496916,   0.7442961705,   0.7952400393,   -0.6062947138,
    -0.6462007402,   -0.7631674805,  -0.2733598753,  0.9619118351,   0.9669590226,
    -0.254931851,    -0.9792894595,  0.2024651934,   -0.5369502995,  -0.8436138784,
    -0.270036471,    -0.9628500944,  -0.6400277131,  0.7683518247,   -0.7854537493,
    -0.6189203566,   0.06005905383,  -0.9981948257,  -0.02455770378, 0.9996984141,
    -0.65983623,     0.751409442,    -0.6253894466,  -0.7803127835,  -0.6210408851,
    -0.7837781695,   0.8348888491,   0.5504185768,   -0.1592275245,  0.9872419133,
    0.8367622488,    0.5475663786,   -0.8675753916,  -0.4973056806,  -0.2022662628,
    -0.9793305667,   0.9399189937,   0.3413975472,   0.9877404807,   -0.1561049093,
    -0.9034455656,   0.4287028224,   0.1269804218,   -0.9919052235,  -0.3819600854,
    0.924178821,     0.9754625894,   0.2201652486,   -0.3204015856,  -0.9472818081,
    -0.9874760884,   0.1577687387,   0.02535348474,  -0.9996785487,  0.4835130794,
    -0.8753371362,   -0.2850799925,  -0.9585037287,  -0.06805516006, -0.99768156,
    -0.7885244045,   -0.6150034663,  0.3185392127,   -0.9479096845,  0.8880043089,
    0.4598351306,    0.6476921488,   -0.7619021462,  0.9820241299,   0.1887554194,
    0.9357275128,    -0.3527237187,  -0.8894895414,  0.4569555293,   0.7922791302,
    0.6101588153,    0.7483818261,   0.6632681526,   -0.7288929755,  -0.6846276581,
    0.8729032783,    -0.4878932944,  0.8288345784,   0.5594937369,   0.08074567077,
    0.9967347374,    0.9799148216,   -0.1994165048,  -0.580730673,   -0.8140957471,
    -0.4700049791,   -0.8826637636,  0.2409492979,   0.9705377045,   0.9437816757,
    -0.3305694308,   -0.8927998638,  -0.4504535528,  -0.8069622304,  0.5906030467,
    0.06258973166,   0.9980393407,   -0.9312597469,  0.3643559849,   0.5777449785,
    0.8162173362,    -0.3360095855,  -0.941858566,   0.697932075,    -0.7161639607,
    -0.002008157227, -0.9999979837,  -0.1827294312,  -0.9831632392,  -0.6523911722,
    0.7578824173,    -0.4302626911,  -0.9027037258,  -0.9985126289,  -0.05452091251,
    -0.01028102172,  -0.9999471489,  -0.4946071129,  0.8691166802,   -0.2999350194,
    0.9539596344,    0.8165471961,   0.5772786819,   0.2697460475,   0.962931498,
    -0.7306287391,   -0.6827749597,  -0.7590952064,  -0.6509796216,  -0.907053853,
    0.4210146171,    -0.5104861064,  -0.8598860013,  0.8613350597,   0.5080373165,
    0.5007881595,    -0.8655698812,  -0.654158152,   0.7563577938,   -0.8382755311,
    -0.545246856,    0.6940070834,   0.7199681717,   0.06950936031,  0.9975812994,
    0.1702942185,    -0.9853932612,  0.2695973274,   0.9629731466,   0.5519612192,
    -0.8338697815,   0.225657487,    -0.9742067022,  0.4215262855,   -0.9068161835,
    0.4881873305,    -0.8727388672,  -0.3683854996,  -0.9296731273,  -0.9825390578,
    0.1860564427,    0.81256471,     0.5828709909,   0.3196460933,   -0.9475370046,
    0.9570913859,    0.2897862643,   -0.6876655497,  -0.7260276109,  -0.9988770922,
    -0.047376731,    -0.1250179027,  0.992154486,    -0.8280133617,  0.560708367,
    0.9324863769,    -0.3612051451,  0.6394653183,   0.7688199442,   -0.01623847064,
    -0.9998681473,   -0.9955014666,  -0.09474613458, -0.81453315,    0.580117012,
    0.4037327978,    -0.9148769469,  0.9944263371,   0.1054336766,   -0.1624711654,
    0.9867132919,    -0.9949487814,  -0.100383875,   -0.6995302564,  0.7146029809,
    0.5263414922,    -0.85027327,    -0.5395221479,  0.841971408,    0.6579370318,
    0.7530729462,    0.01426758847,  -0.9998982128,  -0.6734383991,  0.7392433447,
    0.639412098,     -0.7688642071,  0.9211571421,   0.3891908523,   -0.146637214,
    -0.9891903394,   -0.782318098,   0.6228791163,   -0.5039610839,  -0.8637263605,
    -0.7743120191,   -0.6328039957,
};

template <typename T>
const T FastNoiseLite::Lookup<T>::Gradients3D[] = {
    0, 1, 1,  0, 0,  -1, 1,  0, 0,  1, -1, 0, 0,  -1, -1, 0, 1, 0,  1, 0, -1, 0,  1, 0,
    1, 0, -1, 0, -1, 0,  -1, 0, 1,  1, 0,  0, -1, 1,  0,  0, 1, -1, 0, 0, -1, -1, 0, 0,
    0, 1, 1,  0, 0,  -1, 1,  0, 0,  1, -1, 0, 0,  -1, -1, 0, 1, 0,  1, 0, -1, 0,  1, 0,
    1, 0, -1, 0, -1, 0,  -1, 0, 1,  1, 0,  0, -1, 1,  0,  0, 1, -1, 0, 0, -1, -1, 0, 0,
    0, 1, 1,  0, 0,  -1, 1,  0, 0,  1, -1, 0, 0,  -1, -1, 0, 1, 0,  1, 0, -1, 0,  1, 0,
    1, 0, -1, 0, -1, 0,  -1, 0, 1,  1, 0,  0, -1, 1,  0,  0, 1, -1, 0, 0, -1, -1, 0, 0,
    0, 1, 1,  0, 0,  -1, 1,  0, 0,  1, -1, 0, 0,  -1, -1, 0, 1, 0,  1, 0, -1, 0,  1, 0,
    1, 0, -1, 0, -1, 0,  -1, 0, 1,  1, 0,  0, -1, 1,  0,  0, 1, -1, 0, 0, -1, -1, 0, 0,
    0, 1, 1,  0, 0,  -1, 1,  0, 0,  1, -1, 0, 0,  -1, -1, 0, 1, 0,  1, 0, -1, 0,  1, 0,
    1, 0, -1, 0, -1, 0,  -1, 0, 1,  1, 0,  0, -1, 1,  0,  0, 1, -1, 0, 0, -1, -1, 0, 0,
    1, 1, 0,  0, 0,  -1, 1,  0, -1, 1, 0,  0, 0,  -1, -1, 0
};

template <typename T>
const T FastNoiseLite::Lookup<T>::RandVecs3D[] = {
    -0.7292736885,  -0.6618439697,    0.1735581948,    0,
    0.790292081,    -0.5480887466,    -0.2739291014,   0,
    0.7217578935,   0.6226212466,     -0.3023380997,   0,
    0.565683137,    -0.8208298145,    -0.0790000257,   0,
    0.760049034,    -0.5555979497,    -0.3370999617,   0,
    0.3713945616,   0.5011264475,     0.7816254623,    0,
    -0.1277062463,  -0.4254438999,    -0.8959289049,   0,
    -0.2881560924,  -0.5815838982,    0.7607405838,    0,
    0.5849561111,   -0.662820239,     -0.4674352136,   0,
    0.3307171178,   0.0391653737,     0.94291689,      0,
    0.8712121778,   -0.4113374369,    -0.2679381538,   0,
    0.580981015,    0.7021915846,     0.4115677815,    0,
    0.503756873,    0.6330056931,     -0.5878203852,   0,
    0.4493712205,   0.601390195,      0.6606022552,    0,
    -0.6878403724,  0.09018890807,    -0.7202371714,   0,
    -0.5958956522,  -0.6469350577,    0.475797649,     0,
    -0.5127052122,  0.1946921978,     -0.8361987284,   0,
    -0.9911507142,  -0.05410276466,   -0.1212153153,   0,
    -0.2149721042,  0.9720882117,     -0.09397607749,  0,
    -0.7518650936,  -0.5428057603,    0.3742469607,    0,
    0.5237068895,   0.8516377189,     -0.02107817834,  0,
    0.6333504779,   0.1926167129,     -0.7495104896,   0,
    -0.06788241606, 0.3998305789,     0.9140719259,    0,
    -0.5538628599,  -0.4729896695,    -0.6852128902,   0,
    -0.7261455366,  -0.5911990757,    0.3509933228,    0,
    -0.9229274737,  -0.1782808786,    0.3412049336,    0,
    -0.6968815002,  0.6511274338,     0.3006480328,    0,
    0.9608044783,   -0.2098363234,    -0.1811724921,   0,
    0.06817146062,  -0.9743405129,    0.2145069156,    0,
    -0.3577285196,  -0.6697087264,    -0.6507845481,   0,
    -0.1868621131,  0.7648617052,     -0.6164974636,   0,
    -0.6541697588,  0.3967914832,     0.6439087246,    0,
    0.6993340405,   -0.6164538506,    0.3618239211,    0,
    -0.1546665739,  0.6291283928,     0.7617583057,    0,
    -0.6841612949,  -0.2580482182,    -0.6821542638,   0,
    0.5383980957,   0.4258654885,     0.7271630328,    0,
    -0.5026987823,  -0.7939832935,    -0.3418836993,   0,
    0.3202971715,   0.2834415347,     0.9039195862,    0,
    0.8683227101,   -0.0003762656404, -0.4959995258,   0,
    0.791120031,    -0.08511045745,   0.6057105799,    0,
    -0.04011016052, -0.4397248749,    0.8972364289,    0,
    0.9145119872,   0.3579346169,     -0.1885487608,   0,
    -0.9612039066,  -0.2756484276,    0.01024666929,   0,
    0.6510361721,   -0.2877799159,    -0.7023778346,   0,
    -0.2041786351,  0.7365237271,     0.644859585,     0,
    -0.7718263711,  0.3790626912,     0.5104855816,    0,
    -0.3060082741,  -0.7692987727,    0.5608371729,    0,
    0.454007341,    -0.5024843065,    0.7357899537,    0,
    0.4816795475,   0.6021208291,     -0.6367380315,   0,
    0.6961980369,   -0.3222197429,    0.641469197,     0,
    -0.6532160499,  -0.6781148932,    0.3368515753,    0,
    0.5089301236,   -0.6154662304,    -0.6018234363,   0,
    -0.1635919754,  -0.9133604627,    -0.372840892,    0,
    0.52408019,     -0.8437664109,    0.1157505864,    0,
    0.5902587356,   0.4983817807,     -0.6349883666,   0,
    0.5863227872,   0.494764745,      0.6414307729,    0,
    0.6779335087,   0.2341345225,     0.6968408593,    0,
    0.7177054546,   -0.6858979348,    0.120178631,     0,
    -0.5328819713,  -0.5205125012,    0.6671608058,    0,
    -0.8654874251,  -0.0700727088,    -0.4960053754,   0,
    -0.2861810166,  0.7952089234,     0.5345495242,    0,
    -0.04849529634, 0.9810836427,     -0.1874115585,   0,
    -0.6358521667,  0.6058348682,     0.4781800233,    0,
    0.6254794696,   -0.2861619734,    0.7258696564,    0,
    -0.2585259868,  0.5061949264,     -0.8227581726,   0,
    0.02136306781,  0.5064016808,     -0.8620330371,   0,
    0.200111773,    0.8599263484,     0.4695550591,    0,
    0.4743561372,   0.6014985084,     -0.6427953014,   0,
    0.6622993731,   -0.5202474575,    -0.5391679918,   0,
    0.08084972818,  -0.6532720452,    0.7527940996,    0,
    -0.6893687501,  0.0592860349,     0.7219805347,    0,
    -0.1121887082,  -0.9673185067,    0.2273952515,    0,
    0.7344116094,   0.5979668656,     -0.3210532909,   0,
    0.5789393465,   -0.2488849713,    0.7764570201,    0,
    0.6988182827,   0.3557169806,     -0.6205791146,   0,
    -0.8636845529,  -0.2748771249,    -0.4224826141,   0,
    -0.4247027957,  -0.4640880967,    0.777335046,     0,
    0.5257722489,   -0.8427017621,    0.1158329937,    0,
    0.9343830603,   0.316302472,      -0.1639543925,   0,
    -0.1016836419,  -0.8057303073,    -0.5834887393,   0,
    -0.6529238969,  0.50602126,       -0.5635892736,   0,
    -0.2465286165,  -0.9668205684,    -0.06694497494,  0,
    -0.9776897119,  -0.2099250524,    -0.007368825344, 0,
    0.7736893337,   0.5734244712,     0.2694238123,    0,
    -0.6095087895,  0.4995678998,     0.6155736747,    0,
    0.5794535482,   0.7434546771,     0.3339292269,    0,
    -0.8226211154,  0.08142581855,    0.5627293636,    0,
    -0.510385483,   0.4703667658,     0.7199039967,    0,
    -0.5764971849,  -0.07231656274,   -0.8138926898,   0,
    0.7250628871,   0.3949971505,     -0.5641463116,   0,
    -0.1525424005,  0.4860840828,     -0.8604958341,   0,
    -0.5550976208,  -0.4957820792,    0.667882296,     0,
    -0.1883614327,  0.9145869398,     0.357841725,     0,
    0.7625556724,   -0.5414408243,    -0.3540489801,   0,
    -0.5870231946,  -0.3226498013,    -0.7424963803,   0,
    0.3051124198,   0.2262544068,     -0.9250488391,   0,
    0.6379576059,   0.577242424,      -0.5097070502,   0,
    -0.5966775796,  0.1454852398,     -0.7891830656,   0,
    -0.658330573,   0.6555487542,     -0.3699414651,   0,
    0.7434892426,   0.2351084581,     0.6260573129,    0,
    0.5562114096,   0.8264360377,     -0.0873632843,   0,
    -0.3028940016,  -0.8251527185,    0.4768419182,    0,
    0.1129343818,   -0.985888439,     -0.1235710781,   0,
    0.5937652891,   -0.5896813806,    0.5474656618,    0,
    0.6757964092,   -0.5835758614,    -0.4502648413,   0,
    0.7242302609,   -0.1152719764,    0.6798550586,    0,
    -0.9511914166,  0.0753623979,     -0.2992580792,   0,
    0.2539470961,   -0.1886339355,    0.9486454084,    0,
    0.571433621,    -0.1679450851,    -0.8032795685,   0,
    -0.06778234979, 0.3978269256,     0.9149531629,    0,
    0.6074972649,   0.733060024,      -0.3058922593,   0,
    -0.5435478392,  0.1675822484,     0.8224791405,    0,
    -0.5876678086,  -0.3380045064,    -0.7351186982,   0,
    -0.7967562402,  0.04097822706,    -0.6029098428,   0,
    -0.1996350917,  0.8706294745,     0.4496111079,    0,
    -0.02787660336, -0.9106232682,    -0.4122962022,   0,
    -0.7797625996,  -0.6257634692,    0.01975775581,   0,
    -0.5211232846,  0.7401644346,     -0.4249554471,   0,
    0.8575424857,   0.4053272873,     -0.3167501783,   0,
    0.1045223322,   0.8390195772,     -0.5339674439,   0,
    0.3501822831,   0.9242524096,     -0.1520850155,   0,
    0.1987849858,   0.07647613266,    0.9770547224,    0,
    0.7845996363,   0.6066256811,     -0.1280964233,   0,
    0.09006737436,  -0.9750989929,    -0.2026569073,   0,
    -0.8274343547,  -0.542299559,     0.1458203587,    0,
    -0.3485797732,  -0.415802277,     0.840000362,     0,
    -0.2471778936,  -0.7304819962,    -0.6366310879,   0,
    -0.3700154943,  0.8577948156,     0.3567584454,    0,
    0.5913394901,   -0.548311967,     -0.5913303597,   0,
    0.1204873514,   -0.7626472379,    -0.6354935001,   0,
    0.616959265,    0.03079647928,    0.7863922953,    0,
    0.1258156836,   -0.6640829889,    -0.7369967419,   0,
    -0.6477565124,  -0.1740147258,    -0.7417077429,   0,
    0.6217889313,   -0.7804430448,    -0.06547655076,  0,
    0.6589943422,   -0.6096987708,    0.4404473475,    0,
    -0.2689837504,  -0.6732403169,    -0.6887635427,   0,
    -0.3849775103,  0.5676542638,     0.7277093879,    0,
    0.5754444408,   0.8110471154,     -0.1051963504,   0,
    0.9141593684,   0.3832947817,     0.131900567,     0,
    -0.107925319,   0.9245493968,     0.3654593525,    0,
    0.377977089,    0.3043148782,     0.8743716458,    0,
    -0.2142885215,  -0.8259286236,    0.5214617324,    0,
    0.5802544474,   0.4148098596,     -0.7008834116,   0,
    -0.1982660881,  0.8567161266,     -0.4761596756,   0,
    -0.03381553704, 0.3773180787,     -0.9254661404,   0,
    -0.6867922841,  -0.6656597827,    0.2919133642,    0,
    0.7731742607,   -0.2875793547,    -0.5652430251,   0,
    -0.09655941928, 0.9193708367,     -0.3813575004,   0,
    0.2715702457,   -0.9577909544,    -0.09426605581,  0,
    0.2451015704,   -0.6917998565,    -0.6792188003,   0,
    0.977700782,    -0.1753855374,    0.1155036542,    0,
    -0.5224739938,  0.8521606816,     0.02903615945,   0,
    -0.7734880599,  -0.5261292347,    0.3534179531,    0,
    -0.7134492443,  -0.269547243,     0.6467878011,    0,
    0.1644037271,   0.5105846203,     -0.8439637196,   0,
    0.6494635788,   0.05585611296,    0.7583384168,    0,
    -0.4711970882,  0.5017280509,     -0.7254255765,   0,
    -0.6335764307,  -0.2381686273,    -0.7361091029,   0,
    -0.9021533097,  -0.270947803,     -0.3357181763,   0,
    -0.3793711033,  0.872258117,      0.3086152025,    0,
    -0.6855598966,  -0.3250143309,    0.6514394162,    0,
    0.2900942212,   -0.7799057743,    -0.5546100667,   0,
    -0.2098319339,  0.85037073,       0.4825351604,    0,
    -0.4592603758,  0.6598504336,     -0.5947077538,   0,
    0.8715945488,   0.09616365406,    -0.4807031248,   0,
    -0.6776666319,  0.7118504878,     -0.1844907016,   0,
    0.7044377633,   0.312427597,      0.637304036,     0,
    -0.7052318886,  -0.2401093292,    -0.6670798253,   0,
    0.081921007,    -0.7207336136,    -0.6883545647,   0,
    -0.6993680906,  -0.5875763221,    -0.4069869034,   0,
    -0.1281454481,  0.6419895885,     0.7559286424,    0,
    -0.6337388239,  -0.6785471501,    -0.3714146849,   0,
    0.5565051903,   -0.2168887573,    -0.8020356851,   0,
    -0.5791554484,  0.7244372011,     -0.3738578718,   0,
    0.1175779076,   -0.7096451073,    0.6946792478,    0,
    -0.6134619607,  0.1323631078,     0.7785527795,    0,
    0.6984635305,   -0.02980516237,   -0.715024719,    0,
    0.8318082963,   -0.3930171956,    0.3919597455,    0,
    0.1469576422,   0.05541651717,    -0.9875892167,   0,
    0.708868575,    -0.2690503865,    0.6520101478,    0,
    0.2726053183,   0.67369766,       -0.68688995,     0,
    -0.6591295371,  0.3035458599,     -0.6880466294,   0,
    0.4815131379,   -0.7528270071,    0.4487723203,    0,
    0.9430009463,   0.1675647412,     -0.2875261255,   0,
    0.434802957,    0.7695304522,     -0.4677277752,   0,
    0.3931996188,   0.594473625,      0.7014236729,    0,
    0.7254336655,   -0.603925654,     0.3301814672,    0,
    0.7590235227,   -0.6506083235,    0.02433313207,   0,
    -0.8552768592,  -0.3430042733,    0.3883935666,    0,
    -0.6139746835,  0.6981725247,     0.3682257648,    0,
    -0.7465905486,  -0.5752009504,    0.3342849376,    0,
    0.5730065677,   0.810555537,      -0.1210916791,   0,
    -0.9225877367,  -0.3475211012,    -0.167514036,    0,
    -0.7105816789,  -0.4719692027,    -0.5218416899,   0,
    -0.08564609717, 0.3583001386,     0.929669703,     0,
    -0.8279697606,  -0.2043157126,    0.5222271202,    0,
    0.427944023,    0.278165994,      0.8599346446,    0,
    0.5399079671,   -0.7857120652,    -0.3019204161,   0,
    0.5678404253,   -0.5495413974,    -0.6128307303,   0,
    -0.9896071041,  0.1365639107,     -0.04503418428,  0,
    -0.6154342638,  -0.6440875597,    0.4543037336,    0,
    0.1074204368,   -0.7946340692,    0.5975094525,    0,
    -0.3595449969,  -0.8885529948,    0.28495784,      0,
    -0.2180405296,  0.1529888965,     0.9638738118,    0,
    -0.7277432317,  -0.6164050508,    -0.3007234646,   0,
    0.7249729114,   -0.00669719484,   0.6887448187,    0,
    -0.5553659455,  -0.5336586252,    0.6377908264,    0,
    0.5137558015,   0.7976208196,     -0.3160000073,   0,
    -0.3794024848,  0.9245608561,     -0.03522751494,  0,
    0.8229248658,   0.2745365933,     -0.4974176556,   0,
    -0.5404114394,  0.6091141441,     0.5804613989,    0,
    0.8036581901,   -0.2703029469,    0.5301601931,    0,
    0.6044318879,   0.6832968393,     0.4095943388,    0,
    0.06389988817,  0.9658208605,     -0.2512108074,   0,
    0.1087113286,   0.7402471173,     -0.6634877936,   0,
    -0.713427712,   -0.6926784018,    0.1059128479,    0,
    0.6458897819,   -0.5724548511,    -0.5050958653,   0,
    -0.6553931414,  0.7381471625,     0.159995615,     0,
    0.3910961323,   0.9188871375,     -0.05186755998,  0,
    -0.4879022471,  -0.5904376907,    0.6429111375,    0,
    0.6014790094,   0.7707441366,     -0.2101820095,   0,
    -0.5677173047,  0.7511360995,     0.3368851762,    0,
    0.7858573506,   0.226674665,      0.5753666838,    0,
    -0.4520345543,  -0.604222686,     -0.6561857263,   0,
    0.002272116345, 0.4132844051,     -0.9105991643,   0,
    -0.5815751419,  -0.5162925989,    0.6286591339,    0,
    -0.03703704785, 0.8273785755,     0.5604221175,    0,
    -0.5119692504,  0.7953543429,     -0.3244980058,   0,
    -0.2682417366,  -0.9572290247,    -0.1084387619,   0,
    -0.2322482736,  -0.9679131102,    -0.09594243324,  0,
    0.3554328906,   -0.8881505545,    0.2913006227,    0,
    0.7346520519,   -0.4371373164,    0.5188422971,    0,
    0.9985120116,   0.04659011161,    -0.02833944577,  0,
    -0.3727687496,  -0.9082481361,    0.1900757285,    0,
    0.91737377,     -0.3483642108,    0.1925298489,    0,
    0.2714911074,   0.4147529736,     -0.8684886582,   0,
    0.5131763485,   -0.7116334161,    0.4798207128,    0,
    -0.8737353606,  0.18886992,       -0.4482350644,   0,
    0.8460043821,   -0.3725217914,    0.3814499973,    0,
    0.8978727456,   -0.1780209141,    -0.4026575304,   0,
    0.2178065647,   -0.9698322841,    -0.1094789531,   0,
    -0.1518031304,  -0.7788918132,    -0.6085091231,   0,
    -0.2600384876,  -0.4755398075,    -0.8403819825,   0,
    0.572313509,    -0.7474340931,    -0.3373418503,   0,
    -0.7174141009,  0.1699017182,     -0.6756111411,   0,
    -0.684180784,   0.02145707593,    -0.7289967412,   0,
    -0.2007447902,  0.06555605789,    -0.9774476623,   0,
    -0.1148803697,  -0.8044887315,    0.5827524187,    0,
    -0.7870349638,  0.03447489231,    0.6159443543,    0,
    -0.2015596421,  0.6859872284,     0.6991389226,    0,
    -0.08581082512, -0.10920836,      -0.9903080513,   0,
    0.5532693395,   0.7325250401,     -0.396610771,    0,
    -0.1842489331,  -0.9777375055,    -0.1004076743,   0,
    0.0775473789,   -0.9111505856,    0.4047110257,    0,
    0.1399838409,   0.7601631212,     -0.6344734459,   0,
    0.4484419361,   -0.845289248,     0.2904925424,    0
};

#endif