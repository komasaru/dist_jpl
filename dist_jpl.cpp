/***********************************************************
  太陽／月／地球／その他惑星の任意の２星間の距離を計算

    DATE        AUTHOR       VERSION
    2021.04.02  mk-mode.com  1.00 新規作成

  Copyright(C) 2021 mk-mode.com All Rights Reserved.
----------------------------------------------------------
  引数 : [第１] 対象天体番号（必須。 1 - 13）
         [第２] 基準天体番号（必須。 1 - 13）
                 1: 水星            (Mercury)
                 2: 金星            (Venus)
                 3: 地球            (Earth)
                 4: 火星            (Mars)
                 5: 木星            (Jupiter)
                 6: 土星            (Saturn)
                 7: 天王星          (Uranus)
                 8: 海王星          (Neptune)
                 9: 冥王星          (Pluto)
                10: 月              (Moon)
                11: 太陽            (Sun)
                12: 太陽系重心      (Solar system Barycenter)
                13: 地球 - 月の重心 (Earth-Moon Barycenter)
         [第３] ユリウス日（省略可。省略時は現在日時のユリウス日）
***********************************************************/
#include "jpl.hpp"

#include <cstdlib>
#include <ctime>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>

namespace dist_jpl {

// 定数
static constexpr char kAstrs[13][24] = {
  "Mercury", "Venus", "Earth", "Mars", "Jupiter",
  "Saturn", "Uranus", "Neptune", "Pluto", "Moon", "Sun",
  "Solar system Barycenter", "Earth-Moon barycenter"
};                                                 // 天体名称
static constexpr unsigned int kJstOffset = 32400;
static constexpr bool         kFlgKm     = false;   // 単位フラグ
                                                   //    true: km
                                                   //   false: AU
static constexpr bool         kFlgBary   = true;   // 基準フラグ
                                                   //    true: 太陽系重心が基準
                                                   //   false: 太陽が基準
static constexpr char         kKm[]      = "km";   // 単位文字列
static constexpr char         kAu[]      = "AU";   //     〃

/*
 * @brief      変換: JST -> UTC
 *
 * @param[in]  JST (timespec)
 * @return     UTC (timespec)
 */
struct timespec jst2utc(struct timespec ts_jst) {
  struct timespec ts;

  try {
    ts.tv_sec  = ts_jst.tv_sec - kJstOffset;
    ts.tv_nsec = ts_jst.tv_nsec;
  } catch (...) {
    throw;
  }

  return ts;
}

/*
 * @brief      GC (グレゴリオ暦) -> JD (ユリウス日)
 *
 * @param[in]  GC (timespec)
 * @return     JD (double)
 */
double gc2jd(struct timespec ts) {
  struct tm t;
  unsigned int year;
  unsigned int month;
  unsigned int day;
  unsigned int hour;
  unsigned int min;
  unsigned int sec;
  double jd;

  try {
    localtime_r(&ts.tv_sec, &t);
    year  = t.tm_year + 1900;
    month = t.tm_mon + 1;
    day   = t.tm_mday;
    hour  = t.tm_hour;
    min   = t.tm_min;
    sec   = t.tm_sec;
    // 1月,2月は前年の13月,14月とする
    if (month < 3) {
      --year;
      month += 12;
    }
    // 日付(整数)部分
    jd = static_cast<int>(365.25 * year)
       + static_cast<int>(year / 400.0)
       - static_cast<int>(year / 100.0)
       + static_cast<int>(30.59 * (month - 2))
       + day
       + 1721088.5;
    // 時間(小数)部分
    jd += (sec / 3600.0 + min / 60.0 + hour) / 24.0;
    // 時間(ナノ秒)部分
    jd += ts.tv_nsec / 1000000000.0 / 3600.0 / 24.0;
  } catch (...) {
    throw;
  }

  return jd;
}

/*
 * @brief      日時文字列生成
 *
 * @param[in]  日時 (timespec)
 * @return     日時文字列 (string)
 */
std::string gen_time_str(struct timespec ts) {
  struct tm t;
  std::stringstream ss;
  std::string str_tm;

  try {
    localtime_r(&ts.tv_sec, &t);
    ss << std::setfill('0')
       << std::setw(4) << t.tm_year + 1900 << "-"
       << std::setw(2) << t.tm_mon + 1     << "-"
       << std::setw(2) << t.tm_mday        << " "
       << std::setw(2) << t.tm_hour        << ":"
       << std::setw(2) << t.tm_min         << ":"
       << std::setw(2) << t.tm_sec         << "."
       << std::setw(3) << ts.tv_nsec / 1000000;
    return ss.str();
  } catch (...) {
    throw;
  }
}

/*
 * @brief   距離計算
 *
 * @param   基準天体から見た対象天体の座標
 * @return  距離 (double)
 */
double calc_dist(double ps[3]) {
  double d = 0.0;
  unsigned int i;

  try {
    for (i = 0; i < 3; ++i) { d += ps[i] * ps[i]; }
    d = std::sqrt(d);
  } catch (...) {
    throw;
  }

  return d;
}

}  // namespace dist_jpl

int main(int argc, char* argv[]) {
  namespace ns = dist_jpl;
  unsigned int    astr_t;  // 天体番号（対象）
  unsigned int    astr_c;  // 天体番号（基準）
  double          jd;      // ユリウス日
  int             ret;     // 関数実行結果
  struct timespec jst;     // 時刻オブジェクト
  std::string     unit;    // 単位（位置／角位置）
  double          d;       // 距離

  try {
    if (argc < 3) {
      std::cout << "[USAGE] ./jpl_calc_430 TARGET_NO CENTER_NO [JULIAN_DAY]"
                << std::endl;
      return EXIT_FAILURE;
    }

    // 天体番号（対象）取得
    astr_t = atoi(argv[1]);
    if (astr_t < 1 || astr_t > 15) {
      std::cout << "[ERROR} !!! TARGET_NO must be between 1 and 15 !!!"
                << std::endl;
      return EXIT_FAILURE;
    }
    // 天体番号（基準）取得
    astr_c = atoi(argv[2]);
    if (astr_c < 0 || astr_c > 13) {
      std::cout << "[ERROR} !!! CENTER_NO must be between 0 and 13 !!!"
                << std::endl;
      return EXIT_FAILURE;
    }
    // ユリウス日取得
    if (argc > 3) {
      jd = atof(argv[3]);
    } else {
      // 現在日時の取得
      ret = timespec_get(&jst, TIME_UTC);
      if (ret != 1) {
        std::cout << "[ERROR] Could not get now time!" << std::endl;
        return EXIT_FAILURE;
      }
      std::cout << jst.tv_sec << "." << jst.tv_nsec << std::endl;
      jd = ns::gc2jd(ns::jst2utc(jst));
    }

    // バイナリファイル読み込み
    ns::Jpl o_jpl(jd, ns::kFlgKm, ns::kFlgBary);
    o_jpl.read_bin();

    // 位置・速度(Positions(Radian), Velocities(Radian/Day)) 計算
    o_jpl.calc_pv(astr_t, astr_c);

    // 距離計算
    d = ns::calc_dist(o_jpl.pos);

    // 結果出力
    // （単位 AU で計算しているので、 km は 1AU を乗じる）
    std::cout << "DISTANCE [ "
              << ns::kAstrs[astr_t - 1] << " <=> "
              << ns::kAstrs[astr_c - 1] << " ] (JD: "
              << std::fixed << std::setprecision(8)
              << jd << ")" << std::endl;
    std::cout << "= "
              << d << " " << ns::kAu << std::endl;
    std::cout << "= "
              << d * o_jpl.au << " " << ns::kKm << std::endl;
  } catch (...) {
    std::cerr << "EXCEPTION!" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

