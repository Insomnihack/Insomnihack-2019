sudo rm /var/lib/mysql/* -rf
sudo mkdir -p /var/lib/mysql
sudo mysqld --initialize
sudo chown mysql:mysql /var/lib/mysql

sudo systemctl stop mysql
sudo mysqld_safe --skip-grant-tables --skip-networking &

mysql -u root -e "flush privileges;"
mysql -u root -e "ALTER USER 'root'@'localhost' IDENTIFIED BY 'password';"

pkill -9 mysqld
sudo systemctl start mysql
