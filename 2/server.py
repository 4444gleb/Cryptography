from bottle import route, run, error, get, post, request, redirect
import json
import smtplib
from random import choice, randint
import string
import hashlib
import datetime


@route('/')
def start():
    redirect('/login')


@get('/login')
def login():
    return '''
        <form action="/login" method="post">
            Имя пользователя: <input name="username" type="text" />
            <input value="Get verification code" type="submit" />
        </form>
    '''


@post('/login')
def login():
    username = request.forms.get('username')

    if username == "":
        return "Поле Username не может быть пустым"

    try:
        # Проверяем, есть ли пользователь с таким логином в базе, и если есть, то получаем его емайл
        info_file = open("users_info.json", "r")
        info_json = json.load(info_file)
        email_address = ""
        for person in info_json["users"]:
            if person["username"] == username:
                email_address = person["email_address"]
        info_file.close()
        if email_address == "":
            return "Пользователь с таким Username не найден"

        # Генерируем одноразовый пароль
        single_use_password = ""
        single_use_password_length = randint(7, 20)
        for i in range(0, single_use_password_length):
            single_use_password += choice(string.ascii_letters)

        # Записываем хеш пароля в базу
        pass_hash = hashlib.md5(single_use_password.encode()).hexdigest()
        info_file = open("users_info.json", "r")
        info_json = json.load(info_file)
        validity = 0
        for person in info_json["users"]:
            if person["username"] == username:
                person["hash"] = pass_hash
                validity = int(person["validity"])
        info_file.close()
        info_file = open("users_info.json", "w")
        json.dump(info_json, info_file)
        info_file.close()

        # Записываем текущий юзернейм и вычисляем время окончания действия пароля
        username_file = open("username.txt", "w")
        username_file.write(username)
        username_file.write('\n')
        username_file.write(str(datetime.datetime.timestamp(datetime.datetime.now() + datetime.timedelta(seconds=validity))))
        username_file.close()

        # Отправляем пользователю письмо с одноразовым паролем
        HOST = "smtp.yandex.ru"
        SUBJECT = "Single use password"
        TO = email_address
        FROM = "serversamosledov@yandex.ru"
        password = 'serversamosledov'
        text = "Password: " + single_use_password
        BODY = "\r\n".join((
            "From: %s" % FROM,
            "To: %s" % TO,
            "Subject: %s" % SUBJECT,
            "",
            text
        ))
        server = smtplib.SMTP(HOST)
        server.starttls()
        server.login(FROM, password)
        server.sendmail(FROM, [TO], BODY)
        server.quit()

    except Exception as e:
        print(e)
        redirect('/some_error')

    redirect('/verification')


@get('/verification')
def verification():

    return '''
        <form action="/verification" method="post">
            Пароль из письма: <input name="password" type="password" />
            <input value="Submit code" type="submit" />
        </form>
    '''


@post('/verification')
def login():
    password = request.forms.get('password')
    if password == "":
        return "Поле Password не может быть пустым"
    try:
        # Получаем имя пользователя и время окончания срока действия пароля
        username_file = open("username.txt", "r")
        username = username_file.readline().strip('\n\r')
        validity = datetime.datetime.fromtimestamp(float(username_file.readline()))
        username_file.close()

        # Проверяем, действителен ли пароль
        if validity < datetime.datetime.now():
            return "Срок действия пароля истек"

        # Сверяем хеши паролей
        info_file = open("users_info.json", "r")
        info_json = json.load(info_file)
        for person in info_json["users"]:
            if person["username"] == username:
                if person["hash"] == hashlib.md5(password.encode()).hexdigest():
                    info_file.close()
                    return stub()
                else:
                    info_file.close()
                    return "Пароли не совпадают"
    except Exception as e:
        print(e)
        redirect('/some_error')
    redirect('/some_error')


def stub():
    return '''
        Страница-заглушка. Аутентификация пройдена
    '''


@get('/some_error')
def some_error():
    return '''
        An error occured. Please try again
    '''


@error(404)
def error404():
    return 'Nothing here, sorry'


# users_info = {
#   "users": [
#       {
#           "username": "user12",
#           "email_address": "gleb.samosledov2@yandex.ru",
#           "hash": "hash1",
#           "validity": 300
#       },
#       {
#           "username": "gleb1997",
#           "email_address": "gleb.samosledov@yandex.ru",
#           "hash": "hash2",
#           "validity": 450
#       }
#    ]
# }
# info_file = open("users_info.json", "w")
# json.dump(users_info, info_file)
# info_file.close()


run(host='localhost', port=8080, debug=True)
