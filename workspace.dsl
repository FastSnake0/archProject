workspace {
    name "SocArt"
    description "Простая социальная сеть"

    # включаем режим с иерархической системой идентификаторов
    !identifiers hierarchical

    # Модель архитектуры
    model {

        # Настраиваем возможность создания вложенных груп
        properties { 
            structurizr.groupSeparator "/"
        }
        

        # Описание компонент модели
        user = person "Пользователь"
        social = softwareSystem "Соцсеть" {
            description "Сервис социальной сети"

            client_service = container "Client service" {
                description "Сервис пользовательского приложения"
            }

            user_service = container "User service" {
                description "Сервис управления пользователями"
            }

            post_service = container "Post service" {
                description "Сервис управления лентой"
            }

            chat_service = container "Сhat service" {
                description "Сервис управления чатом"
            }

            group "Слой данных" {
                user_database = container "User Database" {
                    description "База данных с пользователями"
                    technology "PostgreSQL 15"
                    tags "database"
                }

                user_cache = container "User Cache" {
                    description "Кеш пользовательских данных для ускорения аутентификации"
                    technology "PostgreSQL 15"
                    tags "database"
                }

                content_database = container "Content Database" {
                    description "База данных для хранения ленты и личных сообщений"
                    technology "MongoDB 5"
                    tags "database"
                }

            }

            user_service -> user_cache "Получение данных о пользователях"
            user_service -> user_database "Получение/обновление данных о пользователях"
            user_service -> post_service "API запрос от пользователя в ленту"
            user_service -> chat_service "API запрос от пользователя в чат"

            post_service -> content_database "Получение/обновление данных о ленте"
            chat_service -> content_database "Получение/обновление данных о чате"

            user -> client_service "Просмотр ленты и отправка сообщений. Поиск других пользователей"

            client_service -> user_service "API запрос от пользователя в сервис"

        }
        #1

        deploymentEnvironment "Production" {
            deploymentNode "User Server" {
                containerInstance social.user_service
            }

            deploymentNode "Post Server" {
                containerInstance social.post_service
            }

            deploymentNode "Chat Server" {
                containerInstance social.chat_service
            }

            deploymentNode "databases" {
     
                deploymentNode "Database Server 1" {
                    containerInstance social.user_database
                }

                deploymentNode "Database Server 2" {
                    containerInstance social.content_database
                    instances 3
                }

                deploymentNode "Cache Server" {
                    containerInstance social.user_cache
                }
            }
            
        }
    }

    views {
        themes default

        properties { 
            structurizr.tooltips true
        }


        !script groovy {
            workspace.views.createDefaultViews()
            workspace.views.views.findAll { it instanceof com.structurizr.view.ModelView }.each { it.enableAutomaticLayout() }
        }

        systemContext social {
            autoLayout
            include *
        }

        container social {
            autoLayout
            include *
        }

        deployment social "Production" "deployment" {
             include *
             autoLayout
        }

        dynamic social "UC01" "Добавление нового пользователя" {
            autoLayout
            user -> social.client_service "Создать нового пользователя (логин) (POST /user)"
            social.client_service -> social.user_service "Если логин не занят" 
            social.user_service -> social.user_database "Сохранить данные о пользователе"
            social.user_service -> social.user_cache "Добавить данные о пользователе в кэш"
        }

        dynamic social "UC02" "Поиск пользователя по логину" {
            autoLayout
            user -> social.client_service "Найти пользователя по логину ({login}) (GET /user)"
            social.client_service -> social.user_service
            social.user_service -> social.user_cache "Найти пользователя в кэше"
            social.user_service -> social.user_database "Найти пользователя в бд, если нет в кэше"
            
        }

        dynamic social "UC03" "Поиск пользователя по маске имя и фамилии" {
            autoLayout
            user -> social.client_service "Найти пользователя по маске ({mask}) (GET /user)"
            social.client_service -> social.user_service
            social.user_service -> social.user_cache "Найти пользователя в кэше"
            social.user_service -> social.user_database "Найти пользователя в бд, если нет в кэше"   
        }

        dynamic social "UC11" "Добавление записи на стену" {
            autoLayout
            user -> social.client_service "Создать пост (POST /user/post)"
            social.client_service -> social.user_service "Проверка аутентификации пользователя" 
            social.user_service -> social.post_service
            social.post_service -> social.content_database "Сохранить пост"

        }

        dynamic social "UC12" "Загрузка стены пользователя" {
            autoLayout
            user -> social.client_service "Загрузка постов пользователя (GET /user/post)"
            social.client_service -> social.user_service "Проверка аутентификации пользователя" 
            social.user_service -> social.post_service
            social.post_service -> social.content_database "Загрузить стену пользователя"
        }

        dynamic social "UC21" "Отправка сообщения пользователю" {
            autoLayout
            user -> social.client_service "Отправить сообщение (POST /user/chat)"
            social.client_service -> social.user_service "Проверка аутентификации пользователя" 
            social.user_service -> social.chat_service
            social.chat_service -> social.content_database "Отправить сообщение"
        }

        dynamic social "UC22" "Получение списка сообщения для пользователя" {
            autoLayout
            user -> social.client_service "Отправить сообщение (POST /user/chat)"
            social.client_service -> social.user_service "Проверка аутентификации пользователя" 
            social.user_service -> social.chat_service
            social.chat_service -> social.content_database "Загрузить переписку"
        }


    


        styles {
            element "database" {
                shape cylinder
            }
        }
    }
}