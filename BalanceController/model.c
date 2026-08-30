// 자동 생성된 머신러닝 분류기 모델 파일
// 파이썬에서 추출한 피처 순서: 
// [total_mean, total_std, ch1_mean, ch1_std, ch2_mean, ch2_std, ch3_mean, ch3_std, ch4_mean, ch4_std, front_rear_ratio, front_rear_corr, left_right_ratio, cx_mean, cx_std, cy_mean]

#include <string.h>
void add_vectors(double *v1, double *v2, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] + v2[i];
}
void mul_vector_number(double *v1, double num, int size, double *result) {
    for(int i = 0; i < size; ++i)
        result[i] = v1[i] * num;
}
void score(double * input, double * output) {
    double var0[3];
    double var1[3];
    double var2[3];
    double var3[3];
    double var4[3];
    double var5[3];
    double var6[3];
    double var7[3];
    double var8[3];
    double var9[3];
    double var10[3];
    double var11[3];
    double var12[3];
    double var13[3];
    double var14[3];
    double var15[3];
    if (input[1] <= 1309622.0) {
        if (input[9] <= 122677.97265625) {
            if (input[2] <= -9053.72509765625) {
                memcpy(var15, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[11] <= 0.12695301696658134) {
                    if (input[6] <= -59.600006103515625) {
                        memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[15] <= 462.54515075683594) {
                            memcpy(var15, (double[]){0.6470588235294118, 0.23529411764705882, 0.11764705882352941}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.14285714285714285, 0.8571428571428571, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[4] <= 789559.125) {
                        if (input[8] <= 808135.21875) {
                            memcpy(var15, (double[]){0.1016949152542373, 0.864406779661017, 0.03389830508474576}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.0, 0.5, 0.5}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[7] <= 43324.720703125) {
                            memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[14] <= 23.338065147399902) {
                if (input[2] <= 140252.87109375) {
                    memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[0] <= 7951682.75) {
                        if (input[10] <= 2.203585922718048) {
                            memcpy(var15, (double[]){0.06666666666666667, 0.0, 0.9333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[13] <= 112.51249694824219) {
                            memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[8] <= 37491.6748046875) {
                    if (input[5] <= 158540.48828125) {
                        if (input[13] <= 220.25174617767334) {
                            memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[11] <= 0.9437351524829865) {
                            memcpy(var15, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.6666666666666666, 0.3333333333333333, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[5] <= 157506.1015625) {
                        if (input[10] <= 0.6158785969018936) {
                            memcpy(var15, (double[]){0.7272727272727273, 0.2727272727272727, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[13] <= 354.1702423095703) {
                            memcpy(var15, (double[]){0.0, 0.9285714285714286, 0.07142857142857142}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        if (input[15] <= 319.44775390625) {
            if (input[9] <= 265995.3671875) {
                if (input[13] <= -718.561466217041) {
                    memcpy(var15, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[13] <= 114.87203979492188) {
                if (input[15] <= 509.49171447753906) {
                    if (input[10] <= 0.44573526084423065) {
                        memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= 1068973.96875) {
                            memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var15, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var15, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            }
        }
    }
    double var16[3];
    if (input[1] <= 1548218.9375) {
        if (input[12] <= 5.827489376068115) {
            if (input[15] <= 269.75865173339844) {
                if (input[10] <= 7.4528186321258545) {
                    if (input[4] <= 1107622.40625) {
                        memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[0] <= 3097944.0) {
                        if (input[8] <= 33734.0234375) {
                            memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var16, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[6] <= 1419960.75) {
                    if (input[11] <= 0.9123205244541168) {
                        if (input[5] <= 76690.3359375) {
                            memcpy(var16, (double[]){0.75, 0.25, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.05263157894736842, 0.7368421052631579, 0.21052631578947367}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[13] <= 410.6567840576172) {
                            memcpy(var16, (double[]){0.8421052631578947, 0.15789473684210525, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[5] <= 425992.0) {
                        if (input[9] <= 79840.078125) {
                            memcpy(var16, (double[]){0.16666666666666669, 0.6666666666666667, 0.16666666666666669}, 3 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.0, 0.2857142857142857, 0.7142857142857143}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[1] <= 61141.720703125) {
                if (input[14] <= 43.69034671783447) {
                    memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            } else {
                if (input[2] <= 886877.1875) {
                    if (input[6] <= 210593.60546875) {
                        if (input[15] <= 401.9044189453125) {
                            memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var16, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[2] <= 2092219.25) {
                        memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var16, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        }
    } else {
        if (input[7] <= 1185567.0625) {
            if (input[3] <= 285172.15625) {
                if (input[4] <= 1270527.0625) {
                    memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            } else {
                if (input[15] <= 523.5954284667969) {
                    if (input[5] <= 909267.03125) {
                        if (input[11] <= 0.9664170742034912) {
                            memcpy(var16, (double[]){0.0, 0.2, 0.8}, 3 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 2719716.75) {
                            memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var16, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            memcpy(var16, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var15, var16, 3, var14);
    double var17[3];
    if (input[8] <= 1005466.65625) {
        if (input[8] <= 105948.421875) {
            if (input[0] <= 2790965.25) {
                if (input[15] <= 212.1397476196289) {
                    memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[4] <= -225012.3515625) {
                        if (input[12] <= -2.4497766494750977) {
                            memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.2, 0.8, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[14] <= 12.140878677368164) {
                            memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.6176470588235294, 0.29411764705882354, 0.08823529411764706}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[9] <= 77343.33203125) {
                    if (input[10] <= 4.521158695220947) {
                        memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var17, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[15] <= 147.36934661865234) {
                        memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[7] <= 1072854.15625) {
                            memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[15] <= 437.9227294921875) {
                if (input[14] <= 3106.5859375) {
                    if (input[0] <= 4259851.75) {
                        if (input[12] <= -263.9428958892822) {
                            memcpy(var17, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 1823061.75) {
                            memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var17, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[13] <= 90.0955810546875) {
                    if (input[14] <= 4.806773543357849) {
                        memcpy(var17, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[0] <= 3356949.625) {
                            memcpy(var17, (double[]){0.14285714285714285, 0.6428571428571429, 0.21428571428571427}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[7] <= 487503.984375) {
                        memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        }
    } else {
        if (input[2] <= 2634219.375) {
            if (input[5] <= 95105.4296875) {
                if (input[11] <= -0.6747591495513916) {
                    memcpy(var17, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[3] <= 295902.921875) {
                    if (input[6] <= 2401904.6875) {
                        memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[15] <= 365.8629608154297) {
                            memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[0] <= 9561093.0) {
                        if (input[0] <= 6780660.0) {
                            memcpy(var17, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var17, (double[]){0.0, 0.18181818181818182, 0.8181818181818182}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            memcpy(var17, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var14, var17, 3, var13);
    double var18[3];
    if (input[7] <= 392378.90625) {
        if (input[14] <= 17.287416458129883) {
            if (input[1] <= 263877.0) {
                if (input[6] <= 671083.1875) {
                    if (input[14] <= 10.712831020355225) {
                        memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[1] <= 70251.595703125) {
                        memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[14] <= 9.8162260055542) {
                            memcpy(var18, (double[]){0.0, 0.7142857142857143, 0.2857142857142857}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[10] <= 0.07367903087288141) {
                    memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[13] <= -592.2343139648438) {
                if (input[3] <= 203297.515625) {
                    if (input[12] <= -1.4382928609848022) {
                        if (input[13] <= -1523.4175415039062) {
                            memcpy(var18, (double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[3] <= 179247.5859375) {
                    if (input[0] <= 1171156.8125) {
                        if (input[7] <= 215998.2421875) {
                            memcpy(var18, (double[]){0.029411764705882353, 0.9117647058823529, 0.058823529411764705}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){0.25, 0.0, 0.75}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[13] <= -47.30944633483887) {
                            memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){0.2777777777777778, 0.3888888888888889, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[12] <= -4.7756946086883545) {
                        if (input[6] <= 578581.75) {
                            memcpy(var18, (double[]){0.25, 0.75, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[14] <= 23.450034141540527) {
                            memcpy(var18, (double[]){0.0, 0.7142857142857143, 0.2857142857142857}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){0.02702702702702703, 0.972972972972973, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        if (input[0] <= 1941607.0) {
            if (input[12] <= -2.3281806111335754) {
                memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            } else {
                if (input[14] <= 5108.861572265625) {
                    if (input[2] <= 181766.15625) {
                        memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[2] <= 255653.75) {
                            memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[12] <= 2.1886351108551025) {
                memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            } else {
                if (input[11] <= 0.9262770414352417) {
                    if (input[14] <= 9.835686922073364) {
                        memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[11] <= 0.9856305420398712) {
                        if (input[12] <= 8.548855781555176) {
                            memcpy(var18, (double[]){0.0, 0.07142857142857142, 0.9285714285714286}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[15] <= 243.78848266601562) {
                            memcpy(var18, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var18, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var13, var18, 3, var12);
    double var19[3];
    if (input[13] <= 75.46193313598633) {
        if (input[14] <= 281.6062316894531) {
            if (input[7] <= 612115.875) {
                if (input[14] <= 12.773489952087402) {
                    if (input[12] <= 5.6692421436309814) {
                        if (input[1] <= 161048.4296875) {
                            memcpy(var19, (double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var19, (double[]){0.0, 0.25, 0.75}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[12] <= -6.463834524154663) {
                        if (input[13] <= -47.30944633483887) {
                            memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var19, (double[]){0.0, 0.6, 0.4}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[8] <= -355064.828125) {
                            memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var19, (double[]){0.07547169811320754, 0.8867924528301887, 0.03773584905660377}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[3] <= 375091.140625) {
                    memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[8] <= 440234.234375) {
                        memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[9] <= 265425.4296875) {
                if (input[9] <= 6686.66455078125) {
                    memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[15] <= 12.078300476074219) {
                        memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[4] <= -187212.796875) {
                            memcpy(var19, (double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        }
    } else {
        if (input[6] <= 477533.21875) {
            if (input[14] <= 26.845773696899414) {
                memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[9] <= 105007.94140625) {
                    memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[13] <= 989.1903533935547) {
                        if (input[9] <= 127451.69921875) {
                            memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[6] <= 375529.78125) {
                            memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[4] <= 1138324.5) {
                if (input[12] <= 0.8418870568275452) {
                    if (input[6] <= 679294.5) {
                        memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var19, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[5] <= 61125.927734375) {
                        memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[1] <= 2153052.0625) {
                    if (input[8] <= 2084241.6875) {
                        if (input[0] <= 9876782.5) {
                            memcpy(var19, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var19, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var12, var19, 3, var11);
    double var20[3];
    if (input[12] <= 6.201298236846924) {
        if (input[7] <= 152340.3984375) {
            if (input[0] <= -2405.675048828125) {
                memcpy(var20, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[14] <= 25.82430362701416) {
                    if (input[14] <= 14.461434364318848) {
                        if (input[8] <= -42320.42578125) {
                            memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var20, (double[]){0.125, 0.5, 0.375}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[12] <= 0.46615152060985565) {
                            memcpy(var20, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var20, (double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -10882.7001953125) {
                        memcpy(var20, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[2] <= 501687.390625) {
                if (input[6] <= 1376851.5) {
                    if (input[15] <= 708.4807739257812) {
                        if (input[11] <= -0.8238875865936279) {
                            memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var20, (double[]){0.75, 0.0, 0.25}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[6] <= 287100.6796875) {
                            memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var20, (double[]){0.6666666666666666, 0.3333333333333333, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[9] <= 234861.7578125) {
                        memcpy(var20, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[15] <= 509.0201721191406) {
                    if (input[3] <= 375814.109375) {
                        if (input[2] <= 628147.15625) {
                            memcpy(var20, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var20, (double[]){0.07142857142857142, 0.7857142857142857, 0.14285714285714285}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[13] <= 35.70814609527588) {
                            memcpy(var20, (double[]){0.25, 0.75, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var20, (double[]){0.0, 0.15151515151515152, 0.8484848484848485}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[8] <= 877873.34375) {
                        memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[8] <= 962876.6875) {
                            memcpy(var20, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        if (input[8] <= 212636.953125) {
            if (input[9] <= 19856.567138671875) {
                memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[12] <= 9.462133407592773) {
                    if (input[13] <= 51.243431091308594) {
                        memcpy(var20, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var20, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            memcpy(var20, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var11, var20, 3, var10);
    double var21[3];
    if (input[0] <= 3794296.625) {
        if (input[15] <= 372.4170227050781) {
            if (input[2] <= 2177522.25) {
                if (input[4] <= 765121.15625) {
                    if (input[5] <= 277686.4375) {
                        if (input[13] <= -11.605949878692627) {
                            memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.13793103448275862, 0.8620689655172413, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[15] <= 320.29188537597656) {
                            memcpy(var21, (double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[10] <= 11.14492416381836) {
                        memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var21, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[14] <= 85.81492614746094) {
                if (input[9] <= 125673.9609375) {
                    if (input[8] <= 172459.30078125) {
                        memcpy(var21, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[7] <= 79138.326171875) {
                            memcpy(var21, (double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.1, 0.9, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[11] <= -0.24125485867261887) {
                        if (input[4] <= -25585.8759765625) {
                            memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.6666666666666666, 0.3333333333333333, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[15] <= 647.8819885253906) {
                            memcpy(var21, (double[]){0.0, 0.08333333333333333, 0.9166666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[4] <= -352061.0) {
                    memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[9] <= 1125.0291061401367) {
                        memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[15] <= 389.7420349121094) {
                            memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.8214285714285715, 0.14285714285714288, 0.03571428571428572}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        if (input[9] <= 226441.546875) {
            if (input[3] <= 255416.515625) {
                memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[12] <= 4.996710777282715) {
                    memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    if (input[10] <= 1.027400642633438) {
                        memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[9] <= 611010.875) {
                if (input[2] <= 2230523.375) {
                    if (input[7] <= 941601.4375) {
                        if (input[2] <= 2057747.25) {
                            memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[8] <= 440234.234375) {
                        if (input[2] <= 2509816.125) {
                            memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[15] <= 321.0683288574219) {
                    if (input[13] <= 80.28764724731445) {
                        memcpy(var21, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var21, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var10, var21, 3, var9);
    double var22[3];
    if (input[0] <= 2929495.75) {
        if (input[7] <= 157719.5546875) {
            if (input[0] <= 2810994.5) {
                if (input[4] <= 613804.15625) {
                    if (input[3] <= 625133.4375) {
                        if (input[2] <= -10882.7001953125) {
                            memcpy(var22, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var22, (double[]){0.0967741935483871, 0.8548387096774194, 0.04838709677419355}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var22, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[11] <= 0.025821387767791748) {
                        memcpy(var22, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var22, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var22, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[0] <= 442380.40625) {
                memcpy(var22, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[8] <= 1049864.375) {
                    if (input[12] <= -3.375475525856018) {
                        if (input[10] <= 0.9521644115447998) {
                            memcpy(var22, (double[]){0.2857142857142857, 0.0, 0.7142857142857143}, 3 * sizeof(double));
                        } else {
                            memcpy(var22, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[11] <= 0.9437351524829865) {
                            memcpy(var22, (double[]){0.5, 0.3125, 0.1875}, 3 * sizeof(double));
                        } else {
                            memcpy(var22, (double[]){0.125, 0.8125, 0.0625}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var22, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        if (input[5] <= 612795.9375) {
            if (input[9] <= 400454.53125) {
                if (input[12] <= 4.037978172302246) {
                    if (input[12] <= 3.078742504119873) {
                        if (input[13] <= 73.7382926940918) {
                            memcpy(var22, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var22, (double[]){0.0, 0.3157894736842105, 0.6842105263157895}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[5] <= 86041.3017578125) {
                            memcpy(var22, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var22, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[9] <= 77775.90234375) {
                        if (input[6] <= 535554.65625) {
                            memcpy(var22, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var22, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[2] <= 2083990.875) {
                            memcpy(var22, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var22, (double[]){0.5, 0.0, 0.5}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var22, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[9] <= 506019.5) {
                if (input[0] <= 5354460.25) {
                    memcpy(var22, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[15] <= 447.7800598144531) {
                        memcpy(var22, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var22, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var22, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
            }
        }
    }
    add_vectors(var9, var22, 3, var8);
    double var23[3];
    if (input[14] <= 23.42937660217285) {
        if (input[11] <= 0.21570759266614914) {
            if (input[1] <= 147971.8515625) {
                if (input[15] <= 77.15858840942383) {
                    memcpy(var23, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    if (input[6] <= -59.600006103515625) {
                        memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[0] <= 3614577.5) {
                            memcpy(var23, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[1] <= 392390.5625) {
                    memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var23, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[8] <= 808135.21875) {
                if (input[13] <= 83.15470504760742) {
                    if (input[8] <= 106297.5234375) {
                        if (input[8] <= -76108.59765625) {
                            memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var23, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[4] <= -129286.22265625) {
                            memcpy(var23, (double[]){0.0, 0.6666666666666666, 0.3333333333333333}, 3 * sizeof(double));
                        } else {
                            memcpy(var23, (double[]){0.0, 0.9473684210526315, 0.05263157894736842}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var23, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            } else {
                if (input[2] <= 2422754.75) {
                    if (input[7] <= 1219131.6875) {
                        if (input[4] <= 2324178.0) {
                            memcpy(var23, (double[]){0.0, 0.03225806451612903, 0.967741935483871}, 3 * sizeof(double));
                        } else {
                            memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        if (input[7] <= 362567.8125) {
            if (input[2] <= 2000096.25) {
                if (input[13] <= -592.1080627441406) {
                    if (input[7] <= 11170.903076171875) {
                        memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[10] <= 0.5750683322548866) {
                            memcpy(var23, (double[]){0.5, 0.5, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var23, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[15] <= 260.03326416015625) {
                        memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[4] <= -349403.703125) {
                            memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var23, (double[]){0.25, 0.42857142857142855, 0.32142857142857145}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                memcpy(var23, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[3] <= 318530.90625) {
                if (input[6] <= 1415262.8125) {
                    if (input[8] <= 392660.484375) {
                        memcpy(var23, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[13] <= 1650.2844543457031) {
                            memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var23, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var23, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[8] <= 43843.84765625) {
                    if (input[8] <= -155310.59765625) {
                        memcpy(var23, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var23, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var23, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var8, var23, 3, var7);
    double var24[3];
    if (input[6] <= 959070.09375) {
        if (input[15] <= 266.68310546875) {
            if (input[10] <= 4.60945987701416) {
                if (input[1] <= 562.1687545776367) {
                    memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[8] <= 207100.6796875) {
                        if (input[7] <= 157719.5546875) {
                            memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.16666666666666666, 0.8333333333333334, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[4] <= 531558.71875) {
                    memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[14] <= 10.831367492675781) {
                        if (input[5] <= 44247.2734375) {
                            memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        } else {
            if (input[1] <= 300003.40625) {
                if (input[14] <= 22.806538581848145) {
                    memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[6] <= 427389.171875) {
                        if (input[4] <= 9337.949951171875) {
                            memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[10] <= 0.6746699213981628) {
                            memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.0, 0.16666666666666666, 0.8333333333333334}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[12] <= -3.11844539642334) {
                    if (input[10] <= 0.9521644115447998) {
                        if (input[15] <= 435.9743347167969) {
                            memcpy(var24, (double[]){0.0, 0.3333333333333333, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[9] <= 336432.78125) {
                        if (input[14] <= 20.126419067382812) {
                            memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.9130434782608695, 0.08695652173913043, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            }
        }
    } else {
        if (input[13] <= 86.26830673217773) {
            if (input[10] <= 0.29829733073711395) {
                if (input[2] <= 475746.75) {
                    if (input[8] <= 803323.03125) {
                        memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[4] <= -176522.203125) {
                            memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            } else {
                if (input[6] <= 1113254.625) {
                    if (input[0] <= 1656641.4375) {
                        memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[3] <= 308344.1328125) {
                        if (input[12] <= -265.65448474884033) {
                            memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.05555555555555555, 0.9444444444444444, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[10] <= 0.841274231672287) {
                            memcpy(var24, (double[]){0.0, 0.8461538461538461, 0.15384615384615385}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.0, 0.18181818181818182, 0.8181818181818182}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[0] <= 1986299.0625) {
                memcpy(var24, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[12] <= 1.1115291714668274) {
                    if (input[6] <= 2593987.625) {
                        memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[10] <= 0.6346319913864136) {
                        if (input[6] <= 2769364.75) {
                            memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var24, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var24, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            }
        }
    }
    add_vectors(var7, var24, 3, var6);
    double var25[3];
    if (input[9] <= 93910.890625) {
        if (input[12] <= -0.6855870932340622) {
            if (input[11] <= -0.15459556505084038) {
                if (input[14] <= 57.321977615356445) {
                    memcpy(var25, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[4] <= 3262.6500549316406) {
                if (input[10] <= 0.19680997729301453) {
                    memcpy(var25, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    if (input[7] <= 372.0792694091797) {
                        memcpy(var25, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[10] <= 11.569715023040771) {
                    if (input[0] <= 3310659.625) {
                        if (input[15] <= 211.20751953125) {
                            memcpy(var25, (double[]){0.6666666666666666, 0.3333333333333333, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.9375, 0.0625, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[12] <= 2.353227376937866) {
                            memcpy(var25, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.16666666666666666, 0.8333333333333334, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        if (input[14] <= 70.27625274658203) {
            if (input[6] <= 2523967.625) {
                if (input[6] <= 1043093.65625) {
                    if (input[14] <= 36.755797386169434) {
                        if (input[0] <= 3588469.125) {
                            memcpy(var25, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var25, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[13] <= 37.637840270996094) {
                        memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[4] <= -198609.9765625) {
                            memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.0, 0.045454545454545456, 0.9545454545454546}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[14] <= 22.546226501464844) {
                    if (input[5] <= 203648.59375) {
                        memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= 199774.3359375) {
                            memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.0, 0.25, 0.75}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[7] <= 156207.203125) {
                if (input[3] <= 99502.609375) {
                    memcpy(var25, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[14] <= 845.8823547363281) {
                    if (input[11] <= 0.9606267511844635) {
                        if (input[4] <= -437334.375) {
                            memcpy(var25, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.7647058823529411, 0.23529411764705882, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[5] <= 317910.21875) {
                            memcpy(var25, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[2] <= -22759.27490234375) {
                        memcpy(var25, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[12] <= -4.932722806930542) {
                            memcpy(var25, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var25, (double[]){0.1111111111111111, 0.8888888888888888, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var6, var25, 3, var5);
    double var26[3];
    if (input[1] <= 287169.46875) {
        if (input[11] <= 0.319023035466671) {
            if (input[5] <= 83505.078125) {
                if (input[14] <= 69.51256084442139) {
                    memcpy(var26, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var26, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[15] <= 100.62341690063477) {
                    memcpy(var26, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= 55315.541015625) {
                        memcpy(var26, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[5] <= 145103.40625) {
                            memcpy(var26, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var26, (double[]){0.3333333333333333, 0.6666666666666666, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            if (input[10] <= 7.762261152267456) {
                if (input[6] <= 1346341.8125) {
                    memcpy(var26, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[1] <= 158649.7421875) {
                        if (input[13] <= 30.72193092107773) {
                            memcpy(var26, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var26, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var26, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[6] <= 398535.5) {
                    memcpy(var26, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    memcpy(var26, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                }
            }
        }
    } else {
        if (input[12] <= 17.058658599853516) {
            if (input[2] <= 208026.25) {
                if (input[6] <= 247110.0546875) {
                    memcpy(var26, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[4] <= 37596.0498046875) {
                        memcpy(var26, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var26, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[6] <= 488726.28125) {
                    if (input[15] <= 511.32354736328125) {
                        if (input[2] <= 542126.75) {
                            memcpy(var26, (double[]){0.3333333333333333, 0.0, 0.6666666666666666}, 3 * sizeof(double));
                        } else {
                            memcpy(var26, (double[]){0.1, 0.9, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var26, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[13] <= 82.73437881469727) {
                        if (input[5] <= 341329.890625) {
                            memcpy(var26, (double[]){0.03225806451612903, 0.3870967741935484, 0.5806451612903226}, 3 * sizeof(double));
                        } else {
                            memcpy(var26, (double[]){0.08695652173913043, 0.782608695652174, 0.13043478260869565}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[4] <= 1118047.625) {
                            memcpy(var26, (double[]){0.0, 0.027777777777777776, 0.9722222222222222}, 3 * sizeof(double));
                        } else {
                            memcpy(var26, (double[]){0.0, 0.35294117647058826, 0.6470588235294118}, 3 * sizeof(double));
                        }
                    }
                }
            }
        } else {
            memcpy(var26, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
        }
    }
    add_vectors(var5, var26, 3, var4);
    double var27[3];
    if (input[2] <= -10882.7001953125) {
        memcpy(var27, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
    } else {
        if (input[9] <= 122677.97265625) {
            if (input[4] <= 769981.375) {
                if (input[12] <= 6.201298236846924) {
                    if (input[9] <= 149.29866790771484) {
                        memcpy(var27, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[6] <= 947730.0) {
                            memcpy(var27, (double[]){0.05660377358490566, 0.9433962264150944, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var27, (double[]){0.2222222222222222, 0.5925925925925926, 0.18518518518518517}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[8] <= 35649.625) {
                        memcpy(var27, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var27, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[7] <= 43324.720703125) {
                    memcpy(var27, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var27, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[2] <= 1154677.4375) {
                if (input[6] <= 192931.953125) {
                    memcpy(var27, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[10] <= 0.9069695472717285) {
                        if (input[15] <= 487.19317626953125) {
                            memcpy(var27, (double[]){0.3448275862068966, 0.034482758620689655, 0.6206896551724138}, 3 * sizeof(double));
                        } else {
                            memcpy(var27, (double[]){0.12, 0.44, 0.44}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 270262.5625) {
                            memcpy(var27, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var27, (double[]){0.875, 0.125, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[11] <= 0.9966303706169128) {
                    if (input[3] <= 387928.484375) {
                        if (input[5] <= 618980.125) {
                            memcpy(var27, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var27, (double[]){0.0, 0.75, 0.25}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[4] <= 2126666.0) {
                            memcpy(var27, (double[]){0.0, 0.3888888888888889, 0.6111111111111112}, 3 * sizeof(double));
                        } else {
                            memcpy(var27, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var27, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var4, var27, 3, var3);
    double var28[3];
    if (input[8] <= 264173.46875) {
        if (input[13] <= -732.661376953125) {
            if (input[8] <= 94283.69921875) {
                if (input[15] <= 892.421142578125) {
                    memcpy(var28, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    if (input[8] <= -34181.025390625) {
                        memcpy(var28, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[2] <= -50432.0751953125) {
                memcpy(var28, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[12] <= -5.119906187057495) {
                    if (input[5] <= 47300.51953125) {
                        memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[0] <= 1645491.75) {
                            memcpy(var28, (double[]){0.3333333333333333, 0.08333333333333333, 0.5833333333333334}, 3 * sizeof(double));
                        } else {
                            memcpy(var28, (double[]){0.7, 0.3, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[11] <= -0.8120893239974976) {
                        if (input[10] <= 4.499599322676659) {
                            memcpy(var28, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var28, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[1] <= 884054.5) {
                            memcpy(var28, (double[]){0.10294117647058823, 0.8676470588235294, 0.029411764705882353}, 3 * sizeof(double));
                        } else {
                            memcpy(var28, (double[]){0.13333333333333333, 0.4666666666666667, 0.4}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        if (input[2] <= 156696.546875) {
            if (input[12] <= 0.893367350101471) {
                memcpy(var28, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                if (input[0] <= 1268595.15625) {
                    memcpy(var28, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                } else {
                    memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[1] <= 166990.2109375) {
                if (input[5] <= 21955.5185546875) {
                    memcpy(var28, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            } else {
                if (input[11] <= 0.8919636607170105) {
                    if (input[9] <= 85063.1640625) {
                        if (input[0] <= 3491865.25) {
                            memcpy(var28, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[8] <= 952517.65625) {
                            memcpy(var28, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var28, (double[]){0.0, 0.5, 0.5}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[3] <= 387928.484375) {
                        if (input[13] <= 86.42521667480469) {
                            memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var28, (double[]){0.0, 0.375, 0.625}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[0] <= 9561093.0) {
                            memcpy(var28, (double[]){0.0, 0.23076923076923078, 0.7692307692307693}, 3 * sizeof(double));
                        } else {
                            memcpy(var28, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    }
    add_vectors(var3, var28, 3, var2);
    double var29[3];
    if (input[3] <= 316419.34375) {
        if (input[11] <= 0.7020866870880127) {
            if (input[5] <= 103699.59375) {
                if (input[14] <= 23.659486770629883) {
                    if (input[4] <= 756443.9375) {
                        if (input[14] <= 2.8591519594192505) {
                            memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.9523809523809523, 0.047619047619047616, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    }
                } else {
                    if (input[1] <= 497574.171875) {
                        if (input[8] <= -39519.298828125) {
                            memcpy(var29, (double[]){0.6666666666666666, 0.3333333333333333, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.07692307692307693, 0.8461538461538461, 0.07692307692307693}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var29, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    }
                }
            } else {
                if (input[3] <= 145885.90625) {
                    if (input[4] <= 199284.31875610352) {
                        memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                    } else {
                        if (input[3] <= 124040.89453125) {
                            memcpy(var29, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                }
            }
        } else {
            if (input[7] <= 118365.30859375) {
                if (input[0] <= 2864803.5) {
                    if (input[2] <= -4714.5250244140625) {
                        memcpy(var29, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                    } else {
                        if (input[9] <= 242756.8359375) {
                            memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                } else {
                    memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            } else {
                if (input[14] <= 68.36985969543457) {
                    if (input[2] <= 1017270.875) {
                        if (input[1] <= 423428.65625) {
                            memcpy(var29, (double[]){0.0, 0.5, 0.5}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[6] <= 478610.3125) {
                            memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.0, 0.9090909090909091, 0.09090909090909091}, 3 * sizeof(double));
                        }
                    }
                } else {
                    if (input[10] <= 0.636739194393158) {
                        if (input[3] <= 58356.212890625) {
                            memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.9090909090909091, 0.09090909090909091, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[12] <= -2.924965262413025) {
                            memcpy(var29, (double[]){0.0, 0.45454545454545453, 0.5454545454545454}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            }
        }
    } else {
        if (input[1] <= 652736.75) {
            if (input[13] <= -489.5600280761719) {
                memcpy(var29, (double[]){1.0, 0.0, 0.0}, 3 * sizeof(double));
            } else {
                memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
            }
        } else {
            if (input[13] <= 82.73437881469727) {
                if (input[4] <= -12831.82470703125) {
                    memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                } else {
                    if (input[7] <= 575043.65625) {
                        if (input[6] <= 2746857.625) {
                            memcpy(var29, (double[]){0.1, 0.9, 0.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        }
                    } else {
                        if (input[13] <= 67.35728645324707) {
                            memcpy(var29, (double[]){0.0, 0.2727272727272727, 0.7272727272727273}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    }
                }
            } else {
                if (input[9] <= 398919.890625) {
                    if (input[0] <= 9689368.0) {
                        if (input[9] <= 297436.703125) {
                            memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                        } else {
                            memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                        }
                    } else {
                        memcpy(var29, (double[]){0.0, 1.0, 0.0}, 3 * sizeof(double));
                    }
                } else {
                    memcpy(var29, (double[]){0.0, 0.0, 1.0}, 3 * sizeof(double));
                }
            }
        }
    }
    add_vectors(var2, var29, 3, var1);
    mul_vector_number(var1, 0.06666666666666667, 3, var0);
    memcpy(output, var0, 3 * sizeof(double));
}

