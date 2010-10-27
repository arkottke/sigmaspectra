////////////////////////////////////////////////////////////////////////////////////
// This file is part of SigmaSpectra.
// 
// SigmaSpectra is free software: you can redistribute it and/or modify it under the
// terms of the GNU General Public License as published by the Free Software
// Foundation, either version 3 of the License, or (at your option) any later
// version.
// 
// SigmaSpectra is distributed in the hope that it will be useful, but WITHOUT ANY
// WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
// PARTICULAR PURPOSE.  See the GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License along with
// SigmaSpectra.  If not, see <http://www.gnu.org/licenses/>.
// 
// Copyright 2008 Albert Kottke
////////////////////////////////////////////////////////////////////////////////////


#ifndef MOTION_H_
#define MOTION_H_

#include "AbstractMotion.h"

class Motion : public AbstractMotion
{
public:
    Motion(const QString & fileName = "");
    ~Motion();

    virtual QString name() const;
    const QString & fileName() const;
    const QString & component() const;
    const QString & details() const;

    virtual int componentCount() const;

    double dur5_75() const;
    double dur5_95() const;

    const QVector<double> & time() const;
    const QVector<double> & acc() const;
    const QVector<double> & vel() const;
    const QVector<double> & disp() const;

    double pga() const;
    double pgd() const;
    double pgv() const;

    //! Scale the properties of the motion by a factor
    void scaleBy(const double factor);

protected:
    //! Read the file and compute the response spectrum
    void processFile();

    //! Cumulative integration by the trapezoid rule
    static QVector<double> cumtrapz( const QVector<double>& ft, const double dt, const double scale = 1.0);

    //! Find the maximum absolute value of a vector
    static double findMaxAbs( const QVector<double>& );

    /*! Forward Fast Fourier Transform (FFT).
         * \param ts time series
         * \param fas Fourier amplitude spectrum
         */
    static void fft( const QVector<double> & ts, QVector< std::complex<double> >& fas);

    /*! Inverse Fast Fourier Transform (IFFT).
         * \param fas Fourier amplitude spectrum
         * \param ts time series
         */
    static void ifft( const QVector< std::complex<double> >& fas, QVector<double>& ts );

    /*! Compute the acceleration response spectrum.
         * \param damping damping of the oscillators
         * \param period natural periods of the oscillators
         * \param freq frequency of the Fourier amplitude spectrum
         * \param fas Fourier amplitude spectrum
         * \return response spectrum
         */
    static QVector<double> calcRespSpec( const double damping, const QVector<double> & period, const QVector<double> & freq, const QVector<std::complex<double> > & fas );

    /*! Compute a single-degree of freedom transfer function.
         * \param damping damping of the oscillator
         * \param fn natural frequency of the oscillator
         * \param freq frequency of the Fourier amplitude spectrum
         * \param tf reference to the transfer function
         */
    static void calcSdofTf( const double damping, const double fn, const QVector<double>& freq, QVector<std::complex<double> >& tf );

    //! Filename
    QString m_fileName;

    //! Component direction aizmuth (3 digits), N, S, E, W, T, or L -- based on filename
    QString m_comp;

    //! Descriptor of the motion found in the file
    QString m_details;

    //! Time step between data points
    double m_dt;

    //! Time values
    QVector<double> m_time;

    //! Acceleration values in g
    QVector<double> m_acc;

    //! Velocity values in LENGTH/second (based on gravity)
    QVector<double> m_vel;

    //! Displacement values in LENGTH (based on gravity)
    QVector<double> m_disp;

    //! Peak ground acceleration
    double	m_pga;

    //! Peak ground velocity LENGTH/second (based on gravity)
    double m_pgv;

    //! Peak ground displacement LENGTH (based on gravity)
    double m_pgd;

    //! Arias intensity of the motion
    double m_ariasInt;

    //! Durations
    //@{
    //! 5 to 75 percent of the Arias intensity
    double m_dur5_75;
    //! 5 to 95 percent of the Arias intensity
    double m_dur5_95;
    //@}
};
#endif
