@echo off
REM Commit mesajı girilmemişse hata ver ve çık.
if "%~1"=="" (
    echo HATA: Lutfen bir commit mesaji girin.
    echo Kullanim: update "Yaptiginiz degisikligi anlatan bir mesaj"
    goto :eof
)

echo --- Dosyalar sahneye ekleniyor...
git add .

echo --- Degisiklikler kaydediliyor: "%~1"
git commit -m "%~1"

echo --- GitHub'a gonderiliyor...
git push

echo --- Islem tamamlandi.